package SysMDRestImport.entities

interface CommitReference : Record {

    var referencedCommit: Commit?
}