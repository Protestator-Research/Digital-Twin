grammar SysMLv2;

//import KerML;

start: start_element* EOF;

start_element: element | definition_element | usage_element;

startRule: start;

relationship_body: SYMBOL_STATEMENT_DELIMITER | (SYMBOL_CURLY_BRACKET_OPEN owned_annotation SYMBOL_CURLY_BRACKET_CLOSE);
dependency: prefix_metadata_annotation* KEYWORD_DEPENDENCY dependency_declaration relationship_body;
dependency_declaration: (identification KEYWORD_FROM)? qualified_name (SYMBOL_COMMA qualified_name)* KEYWORD_TO qualified_name (SYMBOL_COMMA qualified_name)*;

annotation: qualified_name;
owned_annotation: annotating_element;
annotating_member: annotating_element;
annotating_element: comment | documentation | textual_representation | metadata_feature;

comment: (KEYWORD_COMMENT identification (KEYWORD_ABOUT annotation (SYMBOL_COMMA annotation)*)?)?(KEYWORD_LOCALE STRING_VALUE)? REGULAR_COMMENT;
documentation: KEYWORD_DOC identification? (KEYWORD_LOCALE STRING_VALUE)? REGULAR_COMMENT;
textual_representation: (KEYWORD_REP identification)? KEYWORD_LANGUAGE STRING_VALUE REGULAR_COMMENT;

root_namespace: package_body_element*;
package: prefix_metadata_member* package_declaration package_body;
package_body: SYMBOL_STATEMENT_DELIMITER | (SYMBOL_CURLY_BRACKET_OPEN package_body_element* SYMBOL_CURLY_BRACKET_CLOSE);
package_body_element: package_member |
                      element_filter_member |
                      alias_member |
                      namespace_import;

package_member: definition_element | usage_element;
alias_member: member_prefix KEYWORD_ALIAS ('<'NAME'>')? NAME? KEYWORD_FOR qualified_name relationship_body;

definition_element: package |
                    library_package |
                    annotating_element |
                    dependency |
                    attribute_definition |
                    enumeration_definition |
                    occurrence_definition |
                    individual_definition |
                    item_definition |
                    part_definition |
                    connection_definition |
                    flow_definition |
                    interface_definition |
                    port_definition |
                    action_definition |
                    calculation_definition |
                    state_definition |
                    constraint_definition |
                    requirement_definition |
                    calculation_definition |
                    state_definition |
                    requirement_definition |
                    concern_definition |
                    case_definition |
                    analysis_case_definition |
                    verification_case_definition |
                    view_definition |
                    viewpoint_definition |
                    rendering_definition |
                    metadata_definition |
                    extended_definition;

usage_element: non_occurrence_usage_element | occurrence_usage_element;

basic_definition_prefix: KEYWORD_ABSTRACT | KEYWORD_VARIATION;
definition_extension_keyword: prefix_metadata_member;
definition_prefix: basic_definition_prefix? definition_extension_keyword*;
definition: definition_declaration definition_body;
definition_declaration: identification subsclassification_part?;
definition_body: SYMBOL_STATEMENT_DELIMITER | (SYMBOL_CURLY_BRACKET_OPEN definition_body_item* SYMBOL_CURLY_BRACKET_CLOSE);
definition_body_item: variant_usage_member |
                      non_occurrence_usage_member |
                      definition_member |
                      source_succession_member? occurrence_usage_member |
                      alias_member |
                      namespace_import;
definition_member: member_prefix definition_element;
variant_usage_member: member_prefix KEYWORD_VARIANT variant_usage_element;
non_occurrence_usage_member: member_prefix non_occurrence_usage_element;
occurrence_usage_member: member_prefix occurrence_usage_element;
structure_usage_member: member_prefix structure_usage_element;
behavior_usage_member: member_prefix behavior_usage_element;

feature_direction: KEYWORD_IN | KEYWORD_OUT | KEYWORD_INOUT;
ref_prefix: feature_direction? KEYWORD_DERIVED? (KEYWORD_VARIATION | KEYWORD_CONSTANT | KEYWORD_ABSTRACT KEYWORD_CONSTANT?)?;
basic_usage_prefix: ref_prefix KEYWORD_REF?;
end_usage_prefix: KEYWORD_END? owned_cross_feature_member?;
owned_cross_feature_member: owned_cross_feature;
owned_cross_feature: basic_usage_prefix usage_declaration;
usage_extention_keyword: prefix_metadata_member;
unextended_usage_prefix: end_usage_prefix | basic_usage_prefix;
usage_prefix: unextended_usage_prefix usage_extention_keyword*;
usage: usage_declaration usage_completion;
usage_declaration: identification feature_specialization_part?;
usage_completion: value_part? usage_body;
usage_body: definition_body;

default_reference_usage: ref_prefix usage;
reference_usage: (end_usage_prefix | ref_prefix) KEYWORD_REF usage;
variant_reference: owned_reference_subsetting feature_specilization* usage_body;

non_occurrence_usage_element: default_reference_usage |
                              reference_usage |
                              attribute_usage |
                              enumeration_usage |
                              binding_connector_as_usage |
                              succession_as_usage |
                              extended_usage;
occurrence_usage_element: structure_usage_element | behavior_usage_element;
structure_usage_element: occurrence_usage |
                         individual_usage |
                         portion_usage |
                         event_occurrence_usage |
                         item_usage |
                         part_usage |
                         view_usage |
                         rendering_usage |
                         port_usage |
                         connection_usage |
                         interface_usage |
                         allocation_usage |
                         message |
                         flow_usage |
                         succession_flow_usage;
behavior_usage_element: action_usage |
                        calculation_usage |
                        state_usage |
                        constraint_usage |
                        requirement_usage |
                        concern_usage |
                        case_usage |
                        analysis_case_usage |
                        verification_case_usage |
                        use_case_usage |
                        viewpoint_usage |
                        perform_action_usage |
                        exhibit_state_usage |
                        include_use_case_usage |
                        assert_constriant_usage |
                        satisfy_requirement_usage;

variant_usage_element: variant_reference |
                       reference_usage |
                       attribute_usage |
                       binding_connector_as_usage |
                       succession_as_usage |
                       occurrence_usage |
                       individual_usage |
                       portion_usage |
                       event_occurrence_usage |
                       item_usage |
                       part_usage |
                       view_usage |
                       rendering_usage |
                       port_usage |
                       connection_usage |
                       interface_usage |
                       allocation_usage |
                       message |
                       flow_usage |
                       succession_flow_usage |
                       behavior_usage_element;

subsclassification_part: SPECIALIZES owned_subclassification (SYMBOL_COMMA owned_subclassification)*;

crosses: CROSSES owned_cross_subsetting;
owned_cross_subsetting: qualified_name | owned_feature_chain;
multiplicity_part: owned_multiplicity (KEYWORD_ORDERED? KEYWORD_NONUNIQUE? | KEYWORD_NONUNIQUE KEYWORD_ORDERED?);
owned_multiplicity: multiplicity_range;
multiplicity_range: SYMBOL_SQUARE_BRACKET_OPEN (multiplicity_expression_member SYMBOL_DDOT)? multiplicity_expression_member SYMBOL_SQUARE_BRACKET_CLOSE;

attribute_definition: definition_prefix KEYWORD_ATTRIBUTE KEYWORD_DEF definition;
attribute_usage: usage_prefix? KEYWORD_ATTRIBUTE identification (typed_by identification)? feature_value? usage_body;

enumeration_definition: definition_extension_keyword* KEYWORD_ENUM KEYWORD_DEF definition_declaration enumeration_body;
enumeration_body: SYMBOL_STATEMENT_DELIMITER | (SYMBOL_CURLY_BRACKET_OPEN (annotating_member|enumeration_usage_member)* SYMBOL_CURLY_BRACKET_CLOSE);
enumeration_usage_member: member_prefix enumerated_value;
enumerated_value: KEYWORD_ENUM? usage;
enumeration_usage: usage_prefix KEYWORD_ENUM usage;

occurrence_definition_prefix: basic_definition_prefix? KEYWORD_INDIVIDIAL? definition_extension_keyword*;
occurrence_definition: occurrence_definition_prefix KEYWORD_OCCURRENCE KEYWORD_DEF definition;
individual_definition: basic_definition_prefix? KEYWORD_INDIVIDIAL KEYWORD_DEF definition;

occurrence_usage_prefix: basic_usage_prefix KEYWORD_INDIVIDIAL? portion_kind? usage_extention_keyword*;
occurrence_usage: occurrence_usage_prefix KEYWORD_OCCURRENCE usage;
individual_usage: basic_usage_prefix KEYWORD_INDIVIDIAL portion_kind? usage_extention_keyword* usage;
portion_usage: basic_usage_prefix KEYWORD_INDIVIDIAL? portion_kind usage_extention_keyword* usage;
portion_kind: KEYWORD_SNAPSHOT | KEYWORD_TIMESLICE;
event_occurrence_usage: occurrence_usage_prefix KEYWORD_EVENT ((owned_reference_subsetting feature_specialization_part?)|(KEYWORD_OCCURRENCE usage_declaration)) usage_completion;

source_succession_member: KEYWORD_THEN souce_succession;
souce_succession: source_end_member;
source_end_member: source_end;
source_end: owned_multiplicity?;

item_definition: occurrence_definition_prefix KEYWORD_ITEM KEYWORD_DEF definition;
item_usage: occurrence_usage_prefix KEYWORD_ITEM usage;

part_definition: occurrence_definition_prefix KEYWORD_PART KEYWORD_DEF definition;
part_usage: occurrence_usage_prefix KEYWORD_PART usage;

port_definition: definition_prefix KEYWORD_PORT KEYWORD_DEF definition;
port_usage: occurrence_usage_prefix KEYWORD_PORT usage;
conjungated_port_typing: SYMBOL_CONJUNGATES qualified_name;

connection_definition: occurrence_definition_prefix KEYWORD_CONNECTION KEYWORD_DEF definition;
connection_usage: occurrence_usage_prefix ((KEYWORD_CONNECTION usage_declaration value_part? (KEYWORD_CONNECT connector_part)?)| (KEYWORD_CONNECT connector_part)) usage_body;
connector_part: binary_connector_part | nary_connector_part;
binary_connector_part: connector_end_member KEYWORD_TO connector_end_member;
nary_connector_part: SYMBOL_ROUND_BRACKET_OPEN connector_end_member (SYMBOL_COMMA connector_end_member)+ SYMBOL_ROUND_BRACKET_CLOSE;
connector_end_member: connector_end;
connecotr_end: owned_cross_multiplicity_member? (declared_name = NAME REFERENCES)? owned_reference_subsetting;
owned_cross_multiplicity_member: owned_cross_multiplicity;
owned_cross_multiplicity: owned_multiplicity;

binding_connector_as_usage: usage_prefix (KEYWORD_BINDING usage_declaration)? KEYWORD_BIND connector_end_member SYMBOL_ASSIGN connector_end_member usage_body;
succession_as_usage: usage_prefix (KEYWORD_SUCCSESSION usage_declaration)? KEYWORD_FIRST connector_end_member KEYWORD_THEN connector_end_member usage_body;

interface_definition: occurrence_definition_prefix KEYWORD_INTERFACE KEYWORD_DEF definition_declaration interface_body;
interface_body: SYMBOL_STATEMENT_DELIMITER | (SYMBOL_CURLY_BRACKET_OPEN interface_body_item* SYMBOL_CURLY_BRACKET_CLOSE);
interface_body_item: definition_member |
                     variant_usage_member |
                     interface_non_occurrence_usage_member source_succession_member? |
                     interface_occurrence_usage_member |
                     alias_member |
                     namespace_import;
interface_non_occurrence_usage_member: member_prefix interface_non_occurrence_usage_element;
interface_non_occurrence_usage_element: reference_usage |
                                        attribute_usage |
                                        enumeration_usage |
                                        binding_connector_as_usage |
                                        succession_as_usage;
interface_occurrence_usage_member: member_prefix interface_occurrence_usage_element;
interface_occurrence_usage_element: default_interface_end | structure_usage_element | behavior_usage_element;
default_interface_end: KEYWORD_END usage;

interface_usage: occurrence_usage_prefix KEYWORD_INTERFACE interface_usage_declaration interface_body;
interface_usage_declaration: usage_declaration value_part? (KEYWORD_CONNECT interface_part)? | interface_part;
interface_part: binary_interface_part | nary_interface_part;
binary_interface_part: interface_end_member KEYWORD_TO interface_end_member;
nary_interface_part: SYMBOL_ROUND_BRACKET_OPEN interface_end_member (SYMBOL_COMMA interface_end_member)+ SYMBOL_ROUND_BRACKET_CLOSE;
interface_end_member: interface_end;
interface_end: owned_cross_multiplicity_member? (declared_name=NAME REFERENCES)? owned_reference_subsetting;

allocation_definition: occurrence_definition_prefix KEYWORD_ALLOCATION KEYWORD_DEF definition;
allocation_usage: occurrence_usage_prefix allocation_usage_declaration usage_body;
allocation_usage_declaration: KEYWORD_ALLOCATION usage_declaration (KEYWORD_ALLOCATION connector_part)? | KEYWORD_ALLOCATE connector_part;

flow_definition: occurrence_definition_prefix KEYWORD_FLOW KEYWORD_DEF definition;
message: occurrence_usage_prefix KEYWORD_MESSAGE message_declaration definition_body;
message_declaration: usage_declaration value_part? (KEYWORD_OF flow_payload_feature_member)? (KEYWORD_FROM message_event_member KEYWORD_TO message_event_member)? | message_event_member KEYWORD_TO message_event_member;
message_event_member: message_event;
message_event: owned_reference_subsetting;
flow_usage: occurrence_usage_prefix KEYWORD_FLOW flow_declaration definition_body;
succession_flow_usage: occurrence_usage_prefix KEYWORD_SUCCSESSION KEYWORD_FLOW flow_declaration definition_body;
flow_declaration: usage_declaration value_part? (KEYWORD_OF flow_payload_feature_member)? (KEYWORD_OF flow_end_member KEYWORD_TO flow_end_member)? | flow_end_member KEYWORD_TO flow_end_member;
flow_payload_feature_member: flow_payload_feature;
flow_payload_feature: payload_feature;
payload_feature: identification payload_feature_specialization_part value_part | owned_feature_typing | owned_multiplicity owned_feature_typing;
payload_feature_specialization_part: feature_specilization+ multiplicity_part? feature_specilization* | multiplicity_part feature_specilization+;
flow_end_member: flow_end;
flow_end: flow_end_subsetting? flow_feature_member;
flow_end_subsetting: qualified_name | feature_chain_prefix;
feature_chain_prefix: owned_feature_chaining SYMBOL_DOT (owned_feature_chaining SYMBOL_DOT)+;
flow_feature_member: flow_feature;
flow_feature: flow_feature_redefinition;
flow_feature_redefinition: qualified_name;

action_definition: occurrence_definition_prefix KEYWORD_ACTION KEYWORD_DEF definition_declaration action_body;
action_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN action_body_item* SYMBOL_CURLY_BRACKET_CLOSE;
action_body_item: non_behavior_body_item |
                  initial_node_member action_target_succession_member*|
                  source_succession_member? action_behavior_member action_target_succession_member?|
                  guarded_succession_member;
non_behavior_body_item: namespace_import |
                        alias_member |
                        definition_member |
                        variant_usage_member |
                        source_succession_member? structure_usage_member;
action_behavior_member: behavior_usage_member | action_node_member;
initial_node_member: member_prefix KEYWORD_FIRST qualified_name relationship_body;
action_node_member: member_prefix action_node;
action_target_succession_member: member_prefix action_target_succession;
guarded_succession_member: member_prefix guarded_succession;

action_usage: occurrence_usage_prefix KEYWORD_ACTION action_usage_declaration action_body;
action_usage_declaration: usage_declaration value_part?;
perform_action_usage: occurrence_usage_prefix KEYWORD_PERFORM perform_action_usage_declaration action_body;
perform_action_usage_declaration: owned_reference_subsetting feature_specialization_part? | KEYWORD_ACTION usage_declaration value_part?;
action_node: control_node |
             send_node |
             accept_node |
             assignment_node |
             terminate_node |
             if_node |
             while_loop_node |
             for_loop_node;
action_node_usage_declaration: KEYWORD_ACTION usage_declaration?;
action_node_prefix: occurrence_usage_prefix action_node_usage_declaration?;

control_node: merge_node | decision_node | join_node | fork_node;
control_node_prefix: ref_prefix KEYWORD_INDIVIDIAL? portion_kind? usage_extention_keyword?;
merge_node: control_node_prefix KEYWORD_MERGE? usage_declaration action_body;
decision_node: control_node_prefix KEYWORD_DECIDE? usage_declaration action_body;
join_node: control_node_prefix KEYWORD_DECIDE? usage_declaration action_body;
fork_node: control_node_prefix KEYWORD_FORK? usage_declaration action_body;

accept_node: occurrence_usage_prefix accept_node_declaration action_body;
accept_node_declaration: action_node_usage_declaration? KEYWORD_ACCEPT accept_parameter_part;
accept_parameter_part: payload_parameter_member (KEYWORD_VIA node_parameter_member)?;
payload_parameter_member: payload_parameter;
payload_parameter: payload_feature | identification payload_feature_specialization_part? trigger_value_part;
trigger_value_part: trigger_expression;
trigger_expression: kind=(KEYWORD_AT | KEYWORD_AFTER) argument_member | kind=KEYWORD_WHEN argument_expression_member;
send_node: occurrence_usage_prefix action_usage_declaration? KEYWORD_SEND (node_parameter_member sender_receiver_part? | sender_receiver_part)? action_body;
send_node_declaration: action_node_usage_declaration? KEYWORD_SEND  node_parameter_member sender_receiver_part?;
sender_receiver_part: KEYWORD_VIA node_parameter_member (KEYWORD_TO node_parameter_member)? | KEYWORD_TO node_parameter_member;
node_parameter_member: node_parameter;
node_parameter: feature_binding;
feature_binding: owned_expression;
assignment_node: occurrence_usage_prefix assignment_node_declaration action_body;
assignment_node_declaration: action_node_usage_declaration? KEYWORD_ASSIGN assignment_target_member feature_chain_member SYMBOL_DEF_ASSIGN node_parameter_member;
assignment_target_member: assignment_target_parameter;
assignment_target_parameter: (assignment_target_binding SYMBOL_DOT)?;
assignment_target_binding: non_feature_chain_primary_expression;
feature_chain_member: memberElement = qualified_name | owned_feature_chain;
owned_feature_chain_member: owned_feature_chain;

terminate_node: occurrence_usage_prefix action_node_usage_declaration? KEYWORD_TERMINATE node_parameter_member? action_body;

if_node: action_node_prefix KEYWORD_IF expression_parameter_member action_body_parameter_member (KEYWORD_ELSE (action_body_parameter_member | if_node_parameter_member));
expression_parameter_member: owned_expression;
action_body_parameter_member: action_body_parameter;
action_body_parameter: (KEYWORD_ACTION usage_declaration?)? SYMBOL_CURLY_BRACKET_OPEN action_body_item* SYMBOL_CURLY_BRACKET_CLOSE;
if_node_parameter_member: if_node;
while_loop_node: action_node_prefix (KEYWORD_WHILE expression_parameter_member | KEYWORD_LOOP ) action_body_parameter_member (KEYWORD_UNTIL expression_parameter_member SYMBOL_STATEMENT_DELIMITER);
for_loop_node: action_node_prefix KEYWORD_FOR for_variable_declaration_member KEYWORD_IN node_parameter_member action_body_parameter_member;
for_variable_declaration_member: usage_declaration;
for_variable_declaration: usage_declaration;

action_target_succession: (target_succession | guarded_target_succession | default_target_succession) usage_body;
target_succession: source_end_member KEYWORD_THEN connector_end_member;
guarded_target_succession: guard_expression_member KEYWORD_THEN transition_succession_member;
default_target_succession: KEYWORD_ELSE transition_succession_member;
guarded_succession: (KEYWORD_SUCCSESSION usage_declaration)? KEYWORD_FIRST feature_chain_member guard_expression_member KEYWORD_THEN transition_succession_member usage_body;

state_definition: occurrence_definition_prefix KEYWORD_STATE KEYWORD_DEF definition_declaration state_def_body;
state_def_body: SYMBOL_STATEMENT_DELIMITER | KEYWORD_PARALLEL? SYMBOL_CURLY_BRACKET_OPEN state_body_item SYMBOL_CURLY_BRACKET_CLOSE;
state_body_item: non_behavior_body_item |
                 source_succession_member? behavior_usage_member target_transition_usage_member* |
                 transition_usage_member |
                 entry_action_member entry_transition_member* |
                 do_action_member |
                 exit_action_member;
entry_action_member: member_prefix kind = KEYWORD_ENTRY state_action_usage;
do_action_member: member_prefix kind = KEYWORD_DO state_action_usage;
exit_action_member: member_prefix kind = KEYWORD_EXIT state_action_usage;
entry_transition_member: member_prefix (guarded_target_succession | KEYWORD_THEN target_succession) SYMBOL_STATEMENT_DELIMITER;
state_action_usage: empty_action_usage SYMBOL_STATEMENT_DELIMITER |
                    state_perform_action_uage |
                    state_accept_action_usage |
                    state_send_action_usage |
                    state_assignment_action_usage;
empty_action_usage: {};
state_perform_action_uage: perform_action_usage_declaration action_body;
state_accept_action_usage: accept_node_declaration action_body;
state_send_action_usage: send_node_declaration action_body;
state_assignment_action_usage:assignment_node_declaration action_body;
transition_usage_member: member_prefix transition_usage;
target_transition_usage_member: member_prefix target_transition_usage;

state_usage: occurrence_usage_prefix KEYWORD_STATE action_usage_declaration state_usage_body;
state_usage_body: SYMBOL_STATEMENT_DELIMITER | KEYWORD_PARALLEL? SYMBOL_CURLY_BRACKET_OPEN state_body_item* SYMBOL_CURLY_BRACKET_CLOSE; 
exhibit_state_usage: occurrence_usage_prefix KEYWORD_EXHIBIT (owned_reference_subsetting feature_specialization_part? | KEYWORD_STATE usage_declaration) value_part? state_usage_body;
               
transition_usage: KEYWORD_TRANSISTION (usage_declaration KEYWORD_FROM)? feature_chain_member trigger_action_member? guard_expression_member? effect_behavior_member? KEYWORD_THEN transition_succession_member action_body;
target_transition_usage: (KEYWORD_TRANSISTION trigger_action_member? guard_expression_member? effect_behavior_member? | trigger_action_member guard_expression_member? effect_behavior_member? | guard_expression_member effect_behavior_member?)? KEYWORD_THEN transition_succession_member action_body;
trigger_action_member: KEYWORD_ACCEPT {kind=KEYWORD_TRIGGER} trigger_action;
trigger_action: accept_parameter_part;
guard_expression_member: KEYWORD_IF {kind=KEYWORD_GUARD} owned_expression;
effect_behavior_member: KEYWORD_DO {kind=KEYWORD_TRIGGER} effect_behavior_usage;
effect_behavior_usage: empty_action_usage |
                       transition_perform_action_usage |
                       transition_accept_action_usage |
                       transition_send_action_usage |
                       transition_assignment_action_usage;
transition_perform_action_usage: perform_action_usage_declaration (SYMBOL_CURLY_BRACKET_OPEN action_body_item* SYMBOL_CURLY_BRACKET_CLOSE)?;
transition_accept_action_usage: accept_node_declaration (SYMBOL_CURLY_BRACKET_OPEN action_body_item* SYMBOL_CURLY_BRACKET_CLOSE)?;
transition_send_action_usage: send_node_declaration (SYMBOL_CURLY_BRACKET_OPEN action_body_item* SYMBOL_CURLY_BRACKET_CLOSE)?;
transition_assignment_action_usage: assignment_node_declaration (SYMBOL_CURLY_BRACKET_OPEN action_body_item* SYMBOL_CURLY_BRACKET_CLOSE)?;
transition_succession_member: transition_succession;
transition_succession: connector_end_member;

calculation_definition: occurrence_definition_prefix KEYWORD_CALC KEYWORD_DEF definition_declaration calculation_body;
calculation_usage: occurrence_usage_prefix KEYWORD_CALC action_usage_declaration calculation_body;
calculation_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN calculation_body_part SYMBOL_CURLY_BRACKET_CLOSE;
calculation_body_part: calculation_body_item* result_expression_member?;
calculation_body_item: action_body_item | return_parameter_member;
return_parameter_member: member_prefix? KEYWORD_RETURN usage_element;
result_expression_member: member_prefix? owned_expression;

constraint_definition: occurrence_definition_prefix? definition_declaration calculation_body;
constraint_usage: occurrence_usage_prefix? KEYWORD_CONSTRAINT constraint_usage_declaration calculation_body;
assert_constriant_usage: occurrence_usage_prefix KEYWORD_ASSERT KEYWORD_NOT? (owned_reference_subsetting feature_specialization_part? | KEYWORD_CONSTRAINT constraint_usage_declaration) calculation_body;
constraint_usage_declaration: usage_declaration value_part?;

requirement_definition: occurrence_definition_prefix KEYWORD_REQUIREMENT KEYWORD_DEF definition_declaration requirement_body;
requirement_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN requirement_body_item* SYMBOL_CURLY_BRACKET_CLOSE;
requirement_body_item: definition_body_item |
                       subject_member |
                       requirement_constraint_member |
                       framed_concern_member |
                       requirement_verification_member |
                       actor_member |
                       stakeholder_member;
subject_member: member_prefix subject_usage;
subject_usage: KEYWORD_SUBJECT usage_extention_keyword* usage;
requirement_constraint_member: member_prefix? requriement_kind requirement_constraint_usage;
requriement_kind: KEYWORD_ASSUME {kind = 'assumption'} | KEYWORD_REQUIRE {kind = 'requirement'}; 
requirement_constraint_usage: owned_reference_subsetting feature_specialization_part? requirement_body | (usage_extention_keyword* KEYWORD_CONSTRAINT | usage_extention_keyword+) constraint_usage_declaration calculation_body;
framed_concern_member: member_prefix? KEYWORD_FRAME framed_concern_usage;
framed_concern_usage: owned_reference_subsetting feature_specialization_part? calculation_body | (usage_extention_keyword* KEYWORD_CONSTRAINT | usage_extention_keyword+) constraint_usage_declaration calculation_body;
actor_member: member_prefix actor_usage;
actor_usage: KEYWORD_ACTOR usage_extention_keyword* usage;
stakeholder_member: member_prefix stakeholder_usage;
stakeholder_usage: KEYWORD_STAKEHOLDER usage_extention_keyword;

requirement_usage: occurrence_usage_prefix KEYWORD_REQUIREMENT constraint_usage_declaration requirement_body;
satisfy_requirement_usage: occurrence_usage_prefix KEYWORD_ASSERT KEYWORD_NOT? KEYWORD_SATISFY (owned_reference_subsetting feature_specialization_part? | KEYWORD_REQUIREMENT usage_declaration ) value_part? (KEYWORD_BY satisfaction_subject_member)? requirement_body;
satisfaction_subject_member: satisfaction_parameter;
satisfaction_parameter: satisfaction_feature_value;
satisfaction_feature_value: satisfaction_reference_expression;
satisfaction_reference_expression: feature_chain_member;

concern_definition: occurrence_definition_prefix KEYWORD_CONCERN KEYWORD_DEF definition_declaration requirement_body;
concern_usage: occurrence_usage_prefix KEYWORD_CONCERN constraint_usage_declaration requirement_body;

case_definition: occurrence_definition_prefix KEYWORD_CASE KEYWORD_DEF definition_declaration case_body;
case_usage: occurrence_usage_prefix KEYWORD_CASE constraint_usage_declaration case_body;
case_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN case_body_item* result_expression_member? SYMBOL_CURLY_BRACKET_CLOSE;
case_body_item: action_body_item |
                subject_member |
                actor_member |
                objective_member;
objective_member: member_prefix KEYWORD_OBJECTIVE objective_requirement_usage;
objective_requirement_usage: usage_extention_keyword* constraint_usage_declaration requirement_body;

analysis_case_definition: occurrence_definition_prefix KEYWORD_ANALYSIS KEYWORD_DEF definition_declaration case_body;
analysis_case_usage: occurrence_usage_prefix KEYWORD_ANALYSIS constraint_usage_declaration case_body;

verification_case_definition: occurrence_definition_prefix KEYWORD_VERIFICATION KEYWORD_DEF definition_declaration case_body;
verification_case_usage: occurrence_usage_prefix KEYWORD_VERIFICATION constraint_usage_declaration case_body;

requirement_verification_member: member_prefix KEYWORD_VERIFY {kind = 'requirement'} requirement_verification_usage;
requirement_verification_usage: owned_reference_subsetting feature_specilization* requirement_body | (usage_extention_keyword* KEYWORD_REQUIREMENT | usage_extention_keyword+) constraint_usage_declaration requirement_body;

use_case_definition: occurrence_definition_prefix KEYWORD_USE KEYWORD_CASE KEYWORD_DEF definition_declaration case_body;
use_case_usage: occurrence_usage_prefix KEYWORD_USE KEYWORD_CASE constraint_usage_declaration case_body;
include_use_case_usage: occurrence_usage_prefix KEYWORD_INCLUDE (owned_reference_subsetting feature_specialization_part? | KEYWORD_USE KEYWORD_CASE usage_declaration ) value_part? case_body;

view_definition: occurrence_definition_prefix KEYWORD_VIEW KEYWORD_DEF definition_declaration view_definition_body;
view_definition_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN view_definition_body_item* SYMBOL_CURLY_BRACKET_OPEN;
view_definition_body_item: definition_body_item |
                           element_filter_member |
                           view_rendering_member;
view_rendering_member: member_prefix KEYWORD_RENDER view_rendering_usage;
view_rendering_usage: owned_reference_subsetting feature_specialization_part? usage_body | (usage_extention_keyword* KEYWORD_RENDERING | usage_extention_keyword+) usage;

view_usage: occurrence_usage_prefix KEYWORD_VIEW usage_declaration? value_part? view_body;
view_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN view_body_item* SYMBOL_CURLY_BRACKET_CLOSE;
view_body_item: definition_body_item |
                element_filter_member |
                view_rendering_member |
                expose;
expose: KEYWORD_EXPOSE (membership_expose | namespace_expose) relationship_body;
membership_expose: membership_import;
namespace_expose: namespace_import;

viewpoint_definition: occurrence_definition_prefix KEYWORD_VIEWPOINT KEYWORD_DEF definition_declaration requirement_body;
viewpoint_usage: occurrence_usage_prefix KEYWORD_VIEWPOINT constraint_usage_declaration requirement_body;

rendering_definition: occurrence_definition_prefix KEYWORD_RENDERING KEYWORD_DEF definition;
rendering_usage: occurrence_usage_prefix KEYWORD_RENDERING usage;

metadata_definition: KEYWORD_ABSTRACT? definition_extension_keyword* KEYWORD_METADATA KEYWORD_DEF definition;
prefix_metadata_annotation: SYMBOL_HASHTAG prefix_metadata_usage annotating_element;
prefix_metadata_member: SYMBOL_HASHTAG prefix_metadata_usage;
prefix_metadata_usage: owned_feature_typing;
metadata_usage: usage_extention_keyword* (SYMBOL_AT | KEYWORD_METADATA) metadata_usage_declaration (KEYWORD_ABOUT annotation (SYMBOL_COMMA annotation)*)? metadata_body;
metadata_usage_declaration: (identification (SYMBOL_TYPED_BY | KEYWORD_TYPED KEYWORD_BY))? owned_feature_typing;
metadata_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN (definition_member | metadata_body_usage_member | alias_member | namespace_import) SYMBOL_CURLY_BRACKET_CLOSE;
metadata_body_usage_member: metadata_body_usage;
metadata_body_usage: KEYWORD_REF? REDEFINES? owned_redefinition feature_specialization_part? value_part? metadata_body;
extended_definition: basic_definition_prefix? definition_extension_keyword+ KEYWORD_DEF definition;
extended_usage: unextended_usage_prefix usage_extention_keyword+ usage;

identification: '<' NAME '>' | NAME;
relationship_onwed_elements: relationship_owned_element*;
relationship_owned_element: owned_related_element | owned_annotation;
owned_related_element: non_feature_element | feature_element;

namespace: (prefix_metadata_member)* namespace_declaration namespace_body;
namespace_declaration: KEYWORD_NAMESPACE identification;
namespace_body: SYMBOL_STATEMENT_DELIMITER | (SYMBOL_CURLY_BRACKET_OPEN namespace_body_element SYMBOL_CURLY_BRACKET_CLOSE);
namespace_body_elements: start_element*;
namespace_body_element: namespace_member | alias_member | namespace_import;
member_prefix: (visibility_indicator)?;
visibility_indicator: KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED;
namespace_member: non_feature_member | namespace_feature_member;
non_feature_member: non_feature_element;
namespace_feature_member: member_prefix feature_element;
qualified_name: NAME  (SYMBOL_NAMESPACE_SUBSET NAME)*;

namespace_import: visibility_indicator? KEYWORD_IMPORT KEYWORD_ALL? import_declaration relationship_body?;
import_declaration: membership_import | filter_package;
membership_import: qualified_name (SYMBOL_NAMESPACE_SUBSET? (SYMBOL_DOUBLE_STAR|SYMBOL_STAR)?);
filter_package: membership_import filter_package_member;
filter_package_member: SYMBOL_SQUARE_BRACKET_OPEN owned_expression SYMBOL_SQUARE_BRACKET_CLOSE;

element: annotating_element | non_feature_element | feature_element | additional_options;

non_feature_element: dependency |
                     type |
                     classifier |
                     data_type |
                     namespace |
                     class |
                     structure |
                     metaclass |
                     association |
                     association_structure |
                     interaction |
                     behavior |
                     function |
                     predicate |
                     multiplicity |
                     package |
                     library_package |
                     specialization |
                     conjunction |
                     subclassification |
                     disjoining |
                     feature_inverting |
                     feature_typing |
                     subsetting |
                     redefinition |
                     type_featuring |
                     namespace_import;

feature_element: feature |
                 step |
                 expression |
                 boolean_expression |
                 invariant |
                 connector |
                 binding_connector |
                 succession |
                 item_flow |
                 succession_item_flow;

additional_options: meta_assignment;

type: type_prefix KEYWORD_TYPE type_declaration type_body;
type_prefix: KEYWORD_ABSTRACT? prefix_metadata_member*;
type_declaration: KEYWORD_ALL? identification multiplicity_bounds? (specialization_part | conjugation_part)+ type_relationship_part*;
specialization_part: SPECIALIZES owned_specialization (SYMBOL_COMMA owned_specialization)*;
conjugation_part: CONJUNGATES owned_conjugation;
type_relationship_part: disjoining_part | unioning_part | intersecting_part | differencing_part;
disjoining_part: KEYWORD_DISJOINT KEYWORD_FROM owned_disjoining (SYMBOL_COMMA owned_disjoining)*;
unioning_part: KEYWORD_UNIONS unioning (SYMBOL_COMMA unioning)*;
intersecting_part: KEYWORD_INTERSECTS intersecting (SYMBOL_COMMA intersecting)*;
differencing_part: KEYWORD_DIFFERENCES differencing (SYMBOL_COMMA differencing)*;
type_body: SYMBOL_STATEMENT_DELIMITER | (SYMBOL_CURLY_BRACKET_OPEN type_body_elements SYMBOL_CURLY_BRACKET_CLOSE);
type_body_elements: element*;
type_body_element: element;

specialization: (KEYWORD_SPECILIZATION identification)? KEYWORD_SUBTYPE specific_type SPECIALIZES general_type relationship_body;
owned_specialization: general_type;
specific_type: qualified_name | owned_feature_chain;
general_type: qualified_name | owned_feature_chain;

conjunction: (KEYWORD_CONJUGATION identification)? KEYWORD_CONJUGATE (qualified_name | feature_chain) CONJUNGATES (qualified_name | feature_chain) relationship_body;
owned_conjugation: qualified_name | feature_chain;

disjoining: (KEYWORD_DISJOINING identification)? KEYWORD_DISJOINT (qualified_name | feature_chain) KEYWORD_FROM (qualified_name | feature_chain) relationship_body;
owned_disjoining: qualified_name | feature_chain;

unioning: qualified_name | owned_feature_chain;
intersecting: qualified_name | owned_feature_chain;
differencing: qualified_name | owned_feature_chain;

feature_member: type_feature_member | owned_feature_member;
type_feature_member: member_prefix KEYWORD_MEMBER feature_element;
owned_feature_member: member_prefix feature_element;

classifier: type_prefix? KEYWORD_CLASSIFIER classifier_declaration type_body;
classifier_declaration: KEYWORD_ALL? identification multiplicity_bounds? (superclassing_part | conjugation_part)? type_relationship_part*;
superclassing_part: SPECIALIZES owned_subclassification (SYMBOL_COMMA owned_subclassification)*;
subclassification: (KEYWORD_SPECILIZATION identification)? KEYWORD_SUBCLASSIFIER qualified_name SPECIALIZES qualified_name relationship_body;
owned_subclassification: qualified_name;

feature: feature_prefix? ((KEYWORD_FEATURE feature_declaration) | KEYWORD_FEATURE | prefix_metadata_member) subsettings? feature_assignment? feature_value? type_body;
feature_prefix: (feature_direction)? KEYWORD_ABSTRACT? (KEYWORD_VAR | KEYWORD_COMPOSITE | KEYWORD_PORTION)? KEYWORD_READONLY? KEYWORD_DERIVED? KEYWORD_END? prefix_metadata_member*;
feature_declaration: KEYWORD_ALL? (feature_identification (feature_specialization_part | conjugation_part)? | feature_specialization_part | conjugation_part) feature_relationship_part* type_body?;
feature_identification: SYMBOL_SMALLER NAME SYMBOL_GREATER (NAME)? | NAME;
feature_relationship_part: type_relationship_part | chaining_part | inverting_part | type_featuring_part;
chaining_part: KEYWORD_CHAINS (owned_feature_chaining | feature_chain);
inverting_part: KEYWORD_INVERSE KEYWORD_OF owned_feature_inverting;
type_featuring_part: KEYWORD_FEATURED KEYWORD_BY owned_type_featuring (SYMBOL_COMMA owned_type_featuring)*;
feature_specialization_part: feature_specilization+ multiplicity_part? feature_specilization* | multiplicity_part feature_specilization+;
MULTIPLICITY_PART_ELEMENTS: KEYWORD_ORDERED | KEYWORD_NONUNIQUE;
feature_specilization: typings | subsettings | references | redefinitions;
typings: owned_typed_by (SYMBOL_COMMA owned_feature_typing)*;
owned_typed_by: typed_by owned_feature_typing;
subsettings: subsets owned_subsetting? (SYMBOL_COMMA owned_subsetting)*;
subsets: SUBSETS owned_subsetting;
references: REFERENCES owned_reference_subsetting;
redefinitions: redefines (SYMBOL_COMMA owned_redefinition)?;
redefines: feature_direction? REDEFINES owned_redefinition;

feature_typing: (KEYWORD_SPECILIZATION identification)? KEYWORD_TYPING qualified_name typed_by general_type relationship_body;
owned_feature_typing: general_type;

subsetting: (KEYWORD_SPECILIZATION identification)? KEYWORD_SUBSET? specific_type SUBSETS general_type multiplicity_part? relationship_body;
owned_subsetting: general_type;
owned_reference_subsetting: general_type;

redefinition: feature_direction? (KEYWORD_SPECILIZATION identification)? (KEYWORD_REDEFINITION specific_type)? REDEFINES qualified_name owned_typed_by? multiplicity_part? subsets? feature_assignment? relationship_body;
owned_redefinition: general_type;

owned_feature_chain: feature_chain;
feature_chain: owned_feature_chaining (SYMBOL_DOT owned_feature_chaining)*;
owned_feature_chaining: qualified_name;

feature_inverting: (KEYWORD_INVERTING identification)? KEYWORD_INVERSE (qualified_name | owned_feature_chain) KEYWORD_OF (qualified_name | owned_feature_chain) relationship_body;
owned_feature_inverting: qualified_name | owned_feature_chain;

type_featuring: KEYWORD_FEATURING (identification KEYWORD_OF)? qualified_name KEYWORD_BY qualified_name relationship_body;
owned_type_featuring: qualified_name;

data_type: type_prefix KEYWORD_DATATYPE classifier_declaration type_body;

class: type_prefix KEYWORD_CLASS classifier_declaration type_body;

structure: type_prefix? KEYWORD_STRUCT classifier_declaration type_body;

association: type_prefix KEYWORD_ASSOC classifier_declaration type_body;
association_structure: type_prefix KEYWORD_ASSOC KEYWORD_STRUCT classifier_declaration type_body;

connector: type_prefix? KEYWORD_CONNECTOR connector_declaration type_body;
connector_declaration: feature_declaration (binary_connector_declaration | nary_connector_declaration);
binary_connector_declaration: KEYWORD_ALL? KEYWORD_FROM connector_end_member 'to' connector_end_member;
nary_connector_declaration: SYMBOL_ROUND_BRACKET_OPEN connector_end_member SYMBOL_COMMA connector_end_member (SYMBOL_COMMA connector_end_member)* SYMBOL_ROUND_BRACKET_CLOSE;
connector_end: (NAME REFERENCES)? multiplicity_bounds? owned_reference_subsetting;

binding_connector: feature_prefix KEYWORD_BINDING binding_connector_declaration type_body;
binding_connector_declaration: feature_declaration (KEYWORD_OF connector_end_member SYMBOL_EQUALS connector_end_member)? | (KEYWORD_ALL? (KEYWORD_OF? connector_end_member SYMBOL_EQUALS connector_end_member)?);

succession: feature_prefix KEYWORD_SUCCSESSION succession_declaration type_body;
succession_declaration: feature_declaration (KEYWORD_FIRST connector_end_member KEYWORD_THEN connector_end_member)? | ((KEYWORD_ALL)? (KEYWORD_FIRST? connector_end_member KEYWORD_THEN connector_end_member)?);

behavior: type_prefix KEYWORD_BEHAVIOR classifier_declaration type_body;

step: feature_prefix KEYWORD_STEP feature_declaration value_part? type_body;

function: type_prefix KEYWORD_FUNCTION classifier_declaration function_body;
function_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN function_body_part SYMBOL_CURLY_BRACKET_CLOSE;
function_body_part: (type_body_element | return_feature_member)+ (result_expression_member)?;
return_feature_member: member_prefix KEYWORD_RETURN feature_element;

expression: feature_prefix KEYWORD_EXPR feature_declaration value_part? function_body;

predicate: type_prefix KEYWORD_PREDICATE classifier_declaration function_body;

boolean_expression: feature_prefix KEYWORD_BOOL feature_declaration value_part? function_body;
invariant: feature_prefix KEYWORD_INV (KEYWORD_TRUE | KEYWORD_FALSE)? feature_declaration value_part? function_body;

owned_expression_reference_member: owned_expression_reference;
owned_expression_reference: owned_expression_member;
owned_expression_member: owned_expressions;
owned_expressions: owned_expression+;
owned_expression: conditional_expression |
                  conditional_binary_operator_expression |
                  binary_operator_expression |
                  unary_operator_expression |
                  classification_expression |
                  metaclassification_expression |
                  extend_expression |
                  primary_expression;
eased_owned_expression: primary_expression | extend_expression | metaclassification_expression | unary_operator_expression;
conditional_expression: KEYWORD_IF argument_member SYMBOL_QUESTION argument_expression_member KEYWORD_ELSE argument_expression_member;
conditional_binary_operator_expression: argument_member conditional_binary_operator argument_expression_member;
conditional_binary_operator: SYMBOL_DQUESTION | KEYWORD_OR | KEYWORD_AND | KEYWORD_IMPLIES;
binary_operator_expression:  argument_member binary_operator owned_expressions;
binary_operator: SYMBOL_VERTICAL_LINE | SYMBOL_AND | KEYWORD_XOR | SYMBOL_DDOT | SYMBOL_EQUALS | SYMBOL_NOT_EQUALS | SYMBOL_IFF_EQUALS | SYMBOL_IFF_NOT_EQUALS | SYMBOL_GREATER | SYMBOL_SMALLER | SYMBOL_GREATER_EQUALS | SYMBOL_SMALLER_EQUAL | SYMBOL_PLUS | SYMBOL_MINUS | SYMBOL_STAR | SYMBOL_SLASH | SYMBOL_MOD | SYMBOL_UPPER | SYMBOL_DOUBLE_STAR;
unary_operator_expression: unary_operator owned_expressions;
unary_operator: SYMBOL_PLUS | SYMBOL_MINUS | SYMBOL_CONJUNGATES | KEYWORD_NOT;
classification_expression: argument_member?((classification_test_operator type_reference_member)|(cast_operator type_result_member));
classification: argument_member? (classification_test_operator type_reference_member)|(cast_operator type_result_member);
classification_test_operator: KEYWORD_ISTYPE | KEYWORD_HASTYPE | SYMBOL_AT;
cast_operator: KEYWORD_AS;
metaclassification_expression: metadata_argument_member (metadataclassification_test_operator type_reference_member) | (meta_cast_operator type_result_member);
argument_member: argument;
argument: argument_value;
argument_value: eased_owned_expression;
argument_expression_member: argument_expression;
argument_expression: argument_expression_value;
argument_expression_value: owned_expression_reference;
metadata_argument_member: metadata_argument;
metadata_argument: metadata_value;
metadata_value: metadata_reference;
metadata_reference: qualified_name;
metadataclassification_test_operator: SYMBOL_DOT;
meta_cast_operator: 'meta';
extend_expression: KEYWORD_ALL type_reference_member;
type_reference_member: type_reference;
type_result_member: type_reference;
type_reference: reference_typing;
reference_typing: qualified_name;

primary_expressions: primary_expression+;
primary_expression: feature_chain_expression | non_feature_chain_primary_expression;
primary_argument_value: primary_expression;
primary_argument: primary_argument_value;
primary_argument_member: primary_argument;
non_feature_chain_primary_expression: bracket_expression |
                                index_expression |
                                sequence_expression |
                                select_expression |
                                collect_expression |
                                function_operation_expression |
                                base_expression;
non_feature_chain_primary_argument_value: non_feature_chain_primary_expression;
non_feature_chain_primary_argument: primary_argument;
non_feature_chain_primary_argument_member: primary_argument;
bracket_expression: SYMBOL_SQUARE_BRACKET_OPEN sequence_expression_list_member SYMBOL_SQUARE_BRACKET_CLOSE;
index_expression: SYMBOL_HASHTAG SYMBOL_ROUND_BRACKET_OPEN sequence_expression_list_member SYMBOL_ROUND_BRACKET_CLOSE;
sequence_expression: SYMBOL_ROUND_BRACKET_OPEN sequence_expression_list SYMBOL_ROUND_BRACKET_CLOSE;
sequence_expression_list: owned_expressions SYMBOL_COMMA? | sequence_operator_expression;
sequence_operator_expression: owned_expression_member SYMBOL_COMMA sequence_expression_list_member;
sequence_expression_list_member: sequence_expression_list;
feature_chain_expression: SYMBOL_DOT feature_chain_member;
collect_expression: SYMBOL_DOT body_expression_member;
select_expression: SYMBOL_DOT_QUESTION body_expression_member;
function_operation_expression: '->' reference_typing (body_argument_member |function_reference_argument_member | argument_expression);
body_argument_member: body_argument;
body_argument: body_argument_value;
body_argument_value: body_expression;
body_expression_member: body_expression;
function_reference_argument_member: function_reference_argument;
function_reference_argument: function_reference_member;
function_reference_arugment_value: function_reference_expression;
function_reference_expression: reference_typing;
function_reference_member: function_reference;
function_reference:  reference_typing;


base_expression: null_expression |
                 literal_expression |
                 feature_reference_expression |
                 metadata_access_expression |
                 invocation_expression |
                 body_expression;
null_expression: KEYWORD_NULL | SYMBOL_ROUND_BRACKET_OPEN SYMBOL_ROUND_BRACKET_CLOSE;
feature_reference_expression: feature_reference_member;
feature_reference_member: feature_reference;
feature_reference: qualified_name;

metadata_access_expression: qualified_name SYMBOL_DOT KEYWORD_METADATA;
invocation_expression: internal_invocation_expression;
internal_invocation_expression: (owned_feature_typing | named_argument_list) argument_list;
argument_list: SYMBOL_ROUND_BRACKET_OPEN (owned_expressions positional_argument_list | named_argument_list)? SYMBOL_ROUND_BRACKET_CLOSE;
positional_argument_list: argument_member (SYMBOL_COMMA owned_expressions)*;
named_argument_list: named_argument_member (SYMBOL_COMMA named_argument_member)*;
named_argument_member: named_argument;
named_argument: paramenter_redefinition SYMBOL_EQUALS owned_expressions;
paramenter_redefinition: qualified_name;
body_expression: expression_body_member;
expression_body_member: expression_body;
expression_body: SYMBOL_ROUND_BRACKET_OPEN function_body_part SYMBOL_ROUND_BRACKET_CLOSE;

literal_expression: (KEYWORD_TRUE | KEYWORD_FALSE) |
                    literal_string |
                    literal_integer |
                    literal_real |
                    literal_integer |
                    literal_infinity;
literal_boolean: boolean_value;
boolean_value:  (KEYWORD_TRUE | KEYWORD_FALSE);
literal_string: STRING_VALUE;
literal_integer: DECIMAL_VALUE;
literal_real: real_value;
real_value: (DECIMAL_VALUE? SYMBOL_DOT (DECIMAL_VALUE | EXPONENTIAL_VALUE)) | EXPONENTIAL_VALUE;
literal_infinity: SYMBOL_STAR;

interaction: type_prefix KEYWORD_INTERACTION classifier_declaration type_body;

item_flow: feature_prefix KEYWORD_FLOW item_flow_declaration type_body;
succession_item_flow: feature_prefix KEYWORD_SUCCSESSION KEYWORD_FLOW item_flow_declaration type_body;
item_flow_declaration: (feature_declaration value_part? (KEYWORD_OF item_feature_member)? (KEYWORD_FROM item_flow_end_member KEYWORD_TO item_flow_end_member)? | KEYWORD_ALL? item_flow_end_member KEYWORD_TO item_flow_end_member);
item_feature_member: item_feature;
item_feature: (identification item_feature_specilization_part value_part?) | (owned_feature_typing multiplicity_bounds?) | (multiplicity_bounds owned_feature_typing?);
item_feature_specilization_part: feature_specilization* (multiplicity_part feature_specilization?);
item_flow_end_member: item_flow_end;
item_flow_end: (owned_reference_subsetting SYMBOL_DOT)? item_flow_feature_member;
item_flow_feature_member: item_flow_feature;
item_flow_feature: item_flow_redefinition;
item_flow_redefinition: qualified_name;

value_part: feature_value;
feature_value:(SYMBOL_EQUALS | SYMBOL_DEF_ASSIGN | KEYWORD_DEFAULT (SYMBOL_EQUALS | SYMBOL_DEF_ASSIGN)?) owned_expression;
feature_assignment: SYMBOL_ASSIGN owned_expression;

multiplicity: multiplicity_subset | multiplicity_range;
multiplicity_subset: KEYWORD_MULTIPLICITY identification subsets type_body;
owned_multiplicity_range: multiplicity_bounds;
multiplicity_bounds: SYMBOL_SQUARE_BRACKET_OPEN (multiplicity_expression_member SYMBOL_DDOT)? multiplicity_expression_member SYMBOL_SQUARE_BRACKET_CLOSE;
multiplicity_expression_member: internal_multiplicity_expression_member;
internal_multiplicity_expression_member: (literal_expression | feature_reference_expression);

metaclass: type_prefix? KEYWORD_METACLASS (identification | classifier_declaration) NAME? (SPECIALIZES NAME)? type_body;
prefix_metadata_feature: owned_feature_typing;
metadata_feature: (prefix_metadata_member)* (SYMBOL_AT | KEYWORD_METADATA) metadata_feature_declaration (KEYWORD_ABOUT annotation (SYMBOL_COMMA annotation));
metadata_feature_declaration: (identification (SYMBOL_TYPED_BY | KEYWORD_TYPED KEYWORD_BY))? owned_feature_typing;
metadata_body_element: non_feature_member |
                       metadata_body_feature_member |
                       alias_member |
                       import_declaration;
metadata_body_feature_member: metadata_body_feature;
metadata_body_feature: KEYWORD_FEATURE? (SYMBOL_REDEFINES | KEYWORD_REDEFINES)? owned_redefinition feature_specialization_part? value_part? metadata_body;

library_package: (KEYWORD_STANDARD)? KEYWORD_LIBRARY (prefix_metadata_member)* package_declaration package_body;
package_declaration: KEYWORD_PACKAGE identification?;
element_filter_member: member_prefix KEYWORD_FILTER owned_expression ';';

meta_assignment: qualified_name SYMBOL_ASSIGN identification 'meta' qualified_name ';';

DEFINED_BY: KEYWORD_DEFINED KEYWORD_BY;
CROSSES: SYMBOL_CROSSES | KEYWORD_CROSSES;

//Keywords
KEYWORD_ACCEPT: 'accept';
KEYWORD_ACTION: 'action';
KEYWORD_ACTOR: 'actor';
KEYWORD_AFTER: 'after';
KEYWORD_ALIAS: 'alias';
KEYWORD_ALLOCATE: 'allocate';
KEYWORD_ALLOCATION: 'allocation';
KEYWORD_ANALYSIS: 'analysis';
KEYWORD_ASSIGN: 'assign';
KEYWORD_ASSERT: 'assert';
KEYWORD_ASSUME: 'assume';
KEYWORD_AT: 'at';
KEYWORD_ATTRIBUTE: 'attribute';
KEYWORD_BIND: 'bind';
KEYWORD_BLOCK: 'block';
KEYWORD_CALC: 'calc';
KEYWORD_CASE: 'case';
KEYWORD_CONCERN: 'concern';
KEYWORD_CONNECT: 'connect';
KEYWORD_CONNECTION: 'connection';
KEYWORD_CONSTRAINT: 'constraint';
KEYWORD_CROSSES: 'crosses';
KEYWORD_DECIDE: 'decide';
KEYWORD_DEF: 'def';
KEYWORD_DEFINED: 'defined';
KEYWORD_DERIVED: 'derived';
KEYWORD_DO: 'do';
KEYWORD_ENTRY: 'entry';
KEYWORD_ENUM: 'enum';
KEYWORD_EVENT: 'event';
KEYWORD_EXHIBIT: 'exhibit';
KEYWORD_EXIT: 'exit';
KEYWORD_EXPOSE: 'expose';
KEYWORD_FORK: 'fork';
KEYWORD_FRAME: 'frame';
KEYWORD_GUARD: 'guard';
KEYWORD_INCLUDE: 'include';
KEYWORD_INDIVIDIAL: 'individual';
KEYWORD_INTERFACE:'interface';
KEYWORD_ITEM: 'item';
KEYWORD_JOIN: 'join';
KEYWORD_LOOP: 'loop';
KEYWORD_MERGE: 'merge';
KEYWORD_MESSAGE: 'message';
KEYWORD_NONUNIQUE: 'nonunique';
KEYWORD_OBJECTIVE: 'objective';
KEYWORD_OCCURRENCE: 'occurrence';
KEYWORD_PARALLEL: 'parallel';
KEYWORD_PART:'part';
KEYWORD_PERFORM:'perform';
KEYWORD_PORT: 'port';
KEYWORD_REF: 'ref';
KEYWORD_REFERENCES: 'references';
KEYWORD_RENDER: 'render';
KEYWORD_RENDERING:'rendering';
KEYWORD_REQUIRE: 'require';
KEYWORD_REQUIREMENT: 'requirement';
KEYWORD_SATISFY: 'satisfy';
KEYWORD_SEND: 'send';
KEYWORD_SNAPSHOT: 'snapshot';
KEYWORD_STAKEHOLDER: 'stakeholder';
KEYWORD_STATE: 'state';
KEYWORD_SUBJECT: 'subject';
KEYWORD_TERMINATE: 'terminate';
KEYWORD_TIMESLICE: 'timeslice';
KEYWORD_TRANSISTION: 'transition';
KEYWORD_TRIGGER: 'trigger';
KEYWORD_UNTIL: 'until';
KEYWORD_USE: 'use';
KEYWORD_VARIANT: 'variant';
KEYWORD_VARIATION: 'variation';
KEYWORD_VERIFICATION: 'verification';
KEYWORD_VERIFY: 'verify';
KEYWORD_VIA: 'via';
KEYWORD_VIEW: 'view';
KEYWORD_VIEWPOINT: 'viewpoint';
KEYWORD_WHEN: 'when';
KEYWORD_WHILE: 'while';

//Symbols
SYMBOL_CROSSES: '=>';

typed_by: SYMBOL_TYPED_BY | KEYWORD_TYPED KEYWORD_BY;
SPECIALIZES: SYMBOL_SPECIALIZES | KEYWORD_SPECIALIZES;
SUBSETS:  SYMBOL_SPECIALIZES | KEYWORD_SUBSETS;
REFERENCES: SYMBOL_REFERENCES | KEYWORD_REFERENCES;
REDEFINES: SYMBOL_REDEFINES | KEYWORD_REDEFINES;
CONJUNGATES: SYMBOL_CONJUNGATES | KEYWORD_CONJUGATES;

// Keywords
KEYWORD_ABOUT: 'about';
KEYWORD_ABSTRACT: 'abstract';
KEYWORD_ALL: 'all';
KEYWORD_AND: 'and';
KEYWORD_AS: 'as';
KEYWORD_ASSOC: 'assoc';
KEYWORD_BEHAVIOR: 'behavior';
KEYWORD_BINDING: 'binding';
KEYWORD_BOOL: 'bool';
KEYWORD_BY: 'by';
KEYWORD_CHAINS: 'chains';
KEYWORD_CLASS: 'class';
KEYWORD_CLASSIFIER: 'classifier';
KEYWORD_COMMENT: 'comment';
KEYWORD_COMPOSITE: 'composite';
KEYWORD_CONJUGATE: 'conjugate';
KEYWORD_CONJUGATES: 'conjugates';
KEYWORD_CONJUGATION: 'conjugation';
KEYWORD_CONNECTOR: 'connector';
KEYWORD_DATATYPE: 'datatype';
KEYWORD_DEFAULT: 'default';
KEYWORD_DEPENDENCY: 'dependency';
KEYWORD_DIFFERENCES: 'differences';
KEYWORD_DISJOINING: 'disjoining';
KEYWORD_DISJOINT: 'disjoint';
KEYWORD_DOC: 'doc';
KEYWORD_ELSE: 'else';
KEYWORD_END: 'end';
KEYWORD_EXPR: 'expr';
KEYWORD_FALSE: 'false';
KEYWORD_FEATURE: 'feature';
KEYWORD_FEATURED: 'featured';
KEYWORD_FEATURING: 'featuring';
KEYWORD_FILTER: 'filter';
KEYWORD_FIRST: 'first';
KEYWORD_FLOW: 'flow';
KEYWORD_FOR: 'for';
KEYWORD_FROM: 'from';
KEYWORD_FUNCTION: 'function';
KEYWORD_HASTYPE: 'hastype';
KEYWORD_IF: 'if';
KEYWORD_INTERSECTS:'intersects';
KEYWORD_IMPLIES: 'implies';
KEYWORD_IMPORT: 'import';
KEYWORD_IN: 'in';
KEYWORD_INOUT: 'inout';
KEYWORD_INTERACTION: 'interaction';
KEYWORD_INV: 'inv';
KEYWORD_INVERSE: 'inverse';
KEYWORD_INVERTING: 'inverting';
KEYWORD_ISTYPE: 'istype';
KEYWORD_LANGUAGE: 'language';
KEYWORD_MEMBER: 'member';
KEYWORD_METACLASS: 'metaclass';
KEYWORD_METADATA: 'metadata';
KEYWORD_MULTIPLICITY: 'multiplicity';
KEYWORD_NAMESPACE: 'namespace';
KEYWORD_NOT: 'not';
KEYWORD_NULL: 'null';
KEYWORD_OF: 'of';
KEYWORD_OR: 'or';
KEYWORD_ORDERED: 'ordered';
KEYWORD_OUT: 'out';
KEYWORD_PACKAGE: 'package';
KEYWORD_PORTION: 'potion';
KEYWORD_PREDICATE: 'predicate';
KEYWORD_PRIVATE: 'private';
KEYWORD_PROTECTED: 'protected';
KEYWORD_PUBLIC: 'public';
KEYWORD_READONLY: 'readonly';
KEYWORD_REDEFINES: 'redefines';
KEYWORD_REDEFINITION: 'redefinition';
KEYWORD_REP: 'rep';
KEYWORD_RETURN: 'return';
KEYWORD_SPECILIZATION: 'specialization';
KEYWORD_SPECIALIZES: 'specializes';
KEYWORD_STEP: 'step';
KEYWORD_STRUCT: 'struct';
KEYWORD_SUBCLASSIFIER: 'subclassifier';
KEYWORD_SUBSET: 'subset';
KEYWORD_SUBSETS: 'subsets';
KEYWORD_SUBTYPE: 'subtype';
KEYWORD_SUCCSESSION: 'succession';
KEYWORD_THEN: 'then';
KEYWORD_TO: 'to';
KEYWORD_TRUE: 'true';
KEYWORD_TYPE: 'type';
KEYWORD_TYPED: 'typed';
KEYWORD_TYPING: 'typing';
KEYWORD_UNIONS: 'unions';
KEYWORD_XOR: 'xor';
KEYWORD_VAR: 'var';
KEYWORD_LOCALE: 'locale';
KEYWORD_STANDARD: 'standard';
KEYWORD_LIBRARY: 'library';
KEYWORD_CONSTANT: 'constant';

//Notes and Comments
SINGLE_LINE_NOTE: '//'  ~( '\r' | '\n' )*;
MULTI_LINE_NOTE: SYMBOL_NOTE_BLOCK_START .*? SYMBOL_COMMENT_BLOCK_END;
REGULAR_COMMENT: SYMBOL_COMMENT_BLOCK_START .*? SYMBOL_COMMENT_BLOCK_END;

//Symbols
SYMBOL_COMMENT_BLOCK_START: '/*';
SYMBOL_NOTE_BLOCK_START: '//*';
SYMBOL_COMMENT_BLOCK_END: '*/';
SYMBOL_STATEMENT_DELIMITER: ';';
SYMBOL_STAR:'*';
SYMBOL_NAMESPACE_SUBSET: '::';
SYMBOL_TYPED_BY: ':';
SYMBOL_SPECIALIZES: ':>';
SYMBOL_REFERENCES: '::>';
SYMBOL_REDEFINES: ':>>';
SYMBOL_CONJUNGATES: '~';
SYMBOL_ROUND_BRACKET_OPEN: '(';
SYMBOL_ROUND_BRACKET_CLOSE: ')';
SYMBOL_CURLY_BRACKET_OPEN: '{';
SYMBOL_CURLY_BRACKET_CLOSE: '}';
SYMBOL_SQUARE_BRACKET_OPEN: '[';
SYMBOL_SQUARE_BRACKET_CLOSE: ']';
SYMBOL_COMMA: ',';
SYMBOL_AT: '@';
SYMBOL_HASHTAG: '#';
SYMBOL_MOD: '%';
SYMBOL_AND: '&';
SYMBOL_UPPER: '^';
SYMBOL_VERTICAL_LINE: '|';
SYMBOL_DOUBLE_STAR: '**';
SYMBOL_PLUS: '+';
SYMBOL_MINUS: '-';
SYMBOL_SLASH: '/';
SYMBOL_ARROOW: '->';
SYMBOL_DOT: '.';
SYMBOL_DDOT: '..';
SYMBOL_SMALLER: '<';
SYMBOL_SMALLER_EQUAL: '<=';
SYMBOL_ASSIGN: '=';
SYMBOL_DEF_ASSIGN: ':=';
SYMBOL_EQUALS: '==';
SYMBOL_IFF_EQUALS: '===';
SYMBOL_NOT_EQUALS: '!=';
SYMBOL_IFF_NOT_EQUALS: '!==';
SYMBOL_GREATER: '>';
SYMBOL_GREATER_EQUALS: '>=';
SYMBOL_QUESTION: '?';
SYMBOL_DQUESTION: '??';
SYMBOL_DOT_QUESTION: '.?';

NAME: BASIC_NAME | UNRESTRICTED_NAME;
BASIC_NAME: ('_'|[a-z]|[A-Z]|[0-9])+ ;
UNRESTRICTED_NAME: '\'' .*? '\'';

//Values:
DECIMAL_VALUE: [0-9]+ ;
EXPONENTIAL_VALUE: DECIMAL_VALUE ('e' | 'E') ('+'|'-') DECIMAL_VALUE;
STRING_VALUE: '"' .*? '"';

WS: [ \t\r\n]+ -> skip ;