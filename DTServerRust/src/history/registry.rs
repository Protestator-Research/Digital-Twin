//! Twin registry: topic base name -> twin. Plan 6.2 / 6.3.
//!
//! Collision rule: two twins with the same name share the same topics, their values cannot be told apart.
//! A name that is already owned by another twin id stays with the first one; the later twin is not
//! registered (and not stored) and a warning is logged by the caller.

use std::collections::HashMap;

use uuid::Uuid;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TwinRef {
    pub twin_id: Uuid,
    pub project_id: Uuid,
}

/// A row of the `twin` table.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct TwinRecord {
    pub twin_id: Uuid,
    pub project_id: Uuid,
    pub name: String,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum Registration {
    /// A new twin.
    Added,
    /// Known twin, same name (project may have been updated).
    Refreshed,
    /// Known twin whose name changed; the old name is free again.
    Renamed { old_name: String },
    /// The name belongs to another twin, nothing changed.
    NameTaken { owner: Uuid },
}

/// What [`Registry::apply_database`] did with a row.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum DatabaseMerge {
    Added {
        twin_id: Uuid,
    },
    /// The twin is known in memory; its in-memory name stays.
    KeptMemory {
        twin_id: Uuid,
    },
    /// An in-memory twin lost `name` to the database owner `owner`.
    Evicted {
        twin_id: Uuid,
        name: String,
        owner: Uuid,
    },
}

#[derive(Debug, Default)]
pub struct Registry {
    by_name: HashMap<String, TwinRef>,
    names: HashMap<Uuid, String>,
}

impl Registry {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn len(&self) -> usize {
        self.by_name.len()
    }

    pub fn is_empty(&self) -> bool {
        self.by_name.is_empty()
    }

    pub fn register(&mut self, record: &TwinRecord) -> Registration {
        if let Some(owner) = self.by_name.get(&record.name) {
            if owner.twin_id != record.twin_id {
                return Registration::NameTaken {
                    owner: owner.twin_id,
                };
            }
        }
        let twin = TwinRef {
            twin_id: record.twin_id,
            project_id: record.project_id,
        };
        let previous = self.names.insert(record.twin_id, record.name.clone());
        self.by_name.insert(record.name.clone(), twin);
        match previous {
            None => Registration::Added,
            Some(old) if old == record.name => Registration::Refreshed,
            Some(old) => {
                self.by_name.remove(&old);
                Registration::Renamed { old_name: old }
            }
        }
    }

    /// Merges the rows of the database table into the memory, with one rule that gives the same owner for every
    /// name before and after a restart: **the database owner of a name wins**.
    ///
    /// - A twin id that is already known in memory keeps its (newer) in-memory name and project; the database
    ///   row for it is ignored.
    /// - A database twin whose name is held in memory by a different twin takes the name; the in-memory twin
    ///   loses it (it is removed from the registry, so nothing is recorded for it) and is reported in the result.
    pub fn apply_database(&mut self, records: &[TwinRecord]) -> Vec<DatabaseMerge> {
        let mut events = Vec::new();
        for record in records {
            if self.names.contains_key(&record.twin_id) {
                events.push(DatabaseMerge::KeptMemory {
                    twin_id: record.twin_id,
                });
                continue;
            }
            if let Some(holder) = self.by_name.get(&record.name).copied() {
                // not the same twin (that case was handled above): the database owner takes the name
                self.by_name.remove(&record.name);
                self.names.remove(&holder.twin_id);
                events.push(DatabaseMerge::Evicted {
                    twin_id: holder.twin_id,
                    name: record.name.clone(),
                    owner: record.twin_id,
                });
            }
            self.register(record);
            events.push(DatabaseMerge::Added {
                twin_id: record.twin_id,
            });
        }
        events
    }

    pub fn by_name(&self, name: &str) -> Option<TwinRef> {
        self.by_name.get(name).copied()
    }

    /// Maps `<TwinName>/<elementPath>` to the twin and the element path. The twin name is the first topic
    /// level, so names containing `/` are not supported (the C++ server builds the topics the same way).
    pub fn lookup<'t>(&self, topic: &'t str) -> Option<(TwinRef, &'t str)> {
        let (name, element) = topic.split_once('/')?;
        if element.is_empty() {
            return None;
        }
        Some((self.by_name(name)?, element))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn rec(n: u128, name: &str) -> TwinRecord {
        TwinRecord {
            twin_id: Uuid::from_u128(n),
            project_id: Uuid::from_u128(1000 + n),
            name: name.to_owned(),
        }
    }

    #[test]
    fn first_twin_keeps_the_name() {
        let mut r = Registry::new();
        assert_eq!(r.register(&rec(1, "Pump")), Registration::Added);
        assert_eq!(
            r.register(&rec(2, "Pump")),
            Registration::NameTaken {
                owner: Uuid::from_u128(1)
            }
        );
        assert_eq!(r.len(), 1);
        assert_eq!(r.by_name("Pump").unwrap().twin_id, Uuid::from_u128(1));
        // the same twin again is fine and keeps the mapping
        assert_eq!(r.register(&rec(1, "Pump")), Registration::Refreshed);
        assert_eq!(r.len(), 1);
    }

    #[test]
    fn rename_frees_the_old_name() {
        let mut r = Registry::new();
        r.register(&rec(1, "Pump"));
        assert_eq!(
            r.register(&rec(1, "Pump2")),
            Registration::Renamed {
                old_name: "Pump".into()
            }
        );
        assert!(r.by_name("Pump").is_none());
        assert_eq!(r.register(&rec(2, "Pump")), Registration::Added);
        assert_eq!(r.len(), 2);
        // renaming onto a taken name is refused and changes nothing
        assert_eq!(
            r.register(&rec(1, "Pump")),
            Registration::NameTaken {
                owner: Uuid::from_u128(2)
            }
        );
        assert_eq!(r.by_name("Pump2").unwrap().twin_id, Uuid::from_u128(1));
    }

    #[test]
    fn database_rows_do_not_revert_a_newer_in_memory_name() {
        // twin 1 was renamed in memory before the database became reachable
        let mut r = Registry::new();
        r.register(&rec(1, "Pump"));
        r.register(&rec(1, "PumpNew"));
        let events = r.apply_database(&[rec(1, "Pump"), rec(2, "Valve")]);
        assert_eq!(
            events,
            vec![
                DatabaseMerge::KeptMemory {
                    twin_id: Uuid::from_u128(1)
                },
                DatabaseMerge::Added {
                    twin_id: Uuid::from_u128(2)
                }
            ]
        );
        assert_eq!(r.by_name("PumpNew").unwrap().twin_id, Uuid::from_u128(1));
        assert!(
            r.by_name("Pump").is_none(),
            "the old name is not resurrected"
        );
        assert_eq!(r.by_name("Valve").unwrap().twin_id, Uuid::from_u128(2));
        assert_eq!(r.len(), 2);
    }

    #[test]
    fn database_owner_of_a_name_wins_and_the_loser_stops_being_recorded() {
        // twin B took the name in memory while the database row of that name belongs to twin A
        let mut r = Registry::new();
        r.register(&rec(2, "Pump"));
        let events = r.apply_database(&[rec(1, "Pump")]);
        assert_eq!(
            events,
            vec![
                DatabaseMerge::Evicted {
                    twin_id: Uuid::from_u128(2),
                    name: "Pump".into(),
                    owner: Uuid::from_u128(1)
                },
                DatabaseMerge::Added {
                    twin_id: Uuid::from_u128(1)
                }
            ]
        );
        assert_eq!(r.by_name("Pump").unwrap().twin_id, Uuid::from_u128(1));
        assert_eq!(r.len(), 1);
        // B cannot get it back (same result as after a restart)
        assert_eq!(
            r.register(&rec(2, "Pump")),
            Registration::NameTaken {
                owner: Uuid::from_u128(1)
            }
        );
        // and the outcome equals a fresh start with only the database
        let mut fresh = Registry::new();
        fresh.apply_database(&[rec(1, "Pump")]);
        assert_eq!(fresh.by_name("Pump"), r.by_name("Pump"));
    }

    #[test]
    fn topic_mapping() {
        let mut r = Registry::new();
        r.register(&rec(1, "Pump"));
        let (twin, element) = r.lookup("Pump/Motor/speed").unwrap();
        assert_eq!(twin.twin_id, Uuid::from_u128(1));
        assert_eq!(twin.project_id, Uuid::from_u128(1001));
        assert_eq!(element, "Motor/speed");
        assert_eq!(r.lookup("Pump/x").unwrap().1, "x");
        // unknown twin, no element, case sensitive
        assert!(r.lookup("Other/x").is_none());
        assert!(r.lookup("Pump").is_none());
        assert!(r.lookup("Pump/").is_none());
        assert!(r.lookup("pump/x").is_none());
        assert!(r.lookup("").is_none());
    }
}
