
// Generated from ./SysMLv2.g4 by ANTLR 4.13.2

#pragma once


#include <antlr4-runtime.h>
#include "../Model/Entities/IDigitalTwinElement.h"
#include "../Model/SupportedTypes/SupportedTypes.h"
#include "SysMLv2Listener.h"


/**
 * This class provides an empty implementation of SysMLv2Listener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  SysMLv2BaseListener : public SysMLv2Listener {
public:
	void enterStart(SysMLv2Parser::StartContext * /*ctx*/) override { }
	void exitStart(SysMLv2Parser::StartContext * /*ctx*/) override { }

	void enterStart_element(SysMLv2Parser::Start_elementContext * /*ctx*/) override { }
	void exitStart_element(SysMLv2Parser::Start_elementContext * /*ctx*/) override { }

	void enterStartRule(SysMLv2Parser::StartRuleContext * /*ctx*/) override { }
	void exitStartRule(SysMLv2Parser::StartRuleContext * /*ctx*/) override { }

	void enterRelationship_body(SysMLv2Parser::Relationship_bodyContext * /*ctx*/) override { }
	void exitRelationship_body(SysMLv2Parser::Relationship_bodyContext * /*ctx*/) override { }

	void enterDependency(SysMLv2Parser::DependencyContext * /*ctx*/) override { }
	void exitDependency(SysMLv2Parser::DependencyContext * /*ctx*/) override { }

	void enterDependency_declaration(SysMLv2Parser::Dependency_declarationContext * /*ctx*/) override { }
	void exitDependency_declaration(SysMLv2Parser::Dependency_declarationContext * /*ctx*/) override { }

	void enterAnnotation(SysMLv2Parser::AnnotationContext * /*ctx*/) override { }
	void exitAnnotation(SysMLv2Parser::AnnotationContext * /*ctx*/) override { }

	void enterOwned_annotation(SysMLv2Parser::Owned_annotationContext * /*ctx*/) override { }
	void exitOwned_annotation(SysMLv2Parser::Owned_annotationContext * /*ctx*/) override { }

	void enterAnnotating_member(SysMLv2Parser::Annotating_memberContext * /*ctx*/) override { }
	void exitAnnotating_member(SysMLv2Parser::Annotating_memberContext * /*ctx*/) override { }

	void enterAnnotating_element(SysMLv2Parser::Annotating_elementContext * /*ctx*/) override { }
	void exitAnnotating_element(SysMLv2Parser::Annotating_elementContext * /*ctx*/) override { }

	void enterComment(SysMLv2Parser::CommentContext * /*ctx*/) override { }
	void exitComment(SysMLv2Parser::CommentContext * /*ctx*/) override { }

	void enterDocumentation(SysMLv2Parser::DocumentationContext * /*ctx*/) override { }
	void exitDocumentation(SysMLv2Parser::DocumentationContext * /*ctx*/) override { }

	void enterTextual_representation(SysMLv2Parser::Textual_representationContext * /*ctx*/) override { }
	void exitTextual_representation(SysMLv2Parser::Textual_representationContext * /*ctx*/) override { }

	void enterRoot_namespace(SysMLv2Parser::Root_namespaceContext * /*ctx*/) override { }
	void exitRoot_namespace(SysMLv2Parser::Root_namespaceContext * /*ctx*/) override { }

	void enterPackage(SysMLv2Parser::PackageContext* ctx) override;
	void exitPackage(SysMLv2Parser::PackageContext* ctx) override;

	void enterPackage_body(SysMLv2Parser::Package_bodyContext* /*ctx*/) override { }
	void exitPackage_body(SysMLv2Parser::Package_bodyContext* /*ctx*/) override { }

	void enterPackage_body_element(SysMLv2Parser::Package_body_elementContext* /*ctx*/) override { }
	void exitPackage_body_element(SysMLv2Parser::Package_body_elementContext* /*ctx*/) override { }

	void enterPackage_member(SysMLv2Parser::Package_memberContext * /*ctx*/) override { }
	void exitPackage_member(SysMLv2Parser::Package_memberContext * /*ctx*/) override { }

	void enterAlias_member(SysMLv2Parser::Alias_memberContext * /*ctx*/) override { }
	void exitAlias_member(SysMLv2Parser::Alias_memberContext * /*ctx*/) override { }

	void enterDefinition_element(SysMLv2Parser::Definition_elementContext * /*ctx*/) override { }
	void exitDefinition_element(SysMLv2Parser::Definition_elementContext * /*ctx*/) override { }

	void enterUsage_element(SysMLv2Parser::Usage_elementContext * /*ctx*/) override { }
	void exitUsage_element(SysMLv2Parser::Usage_elementContext * /*ctx*/) override { }

	void enterBasic_definition_prefix(SysMLv2Parser::Basic_definition_prefixContext * /*ctx*/) override { }
	void exitBasic_definition_prefix(SysMLv2Parser::Basic_definition_prefixContext * /*ctx*/) override { }

	void enterDefinition_extension_keyword(SysMLv2Parser::Definition_extension_keywordContext * /*ctx*/) override { }
	void exitDefinition_extension_keyword(SysMLv2Parser::Definition_extension_keywordContext * /*ctx*/) override { }

	void enterDefinition_prefix(SysMLv2Parser::Definition_prefixContext * /*ctx*/) override { }
	void exitDefinition_prefix(SysMLv2Parser::Definition_prefixContext * /*ctx*/) override { }

	void enterDefinition(SysMLv2Parser::DefinitionContext * /*ctx*/) override { }
	void exitDefinition(SysMLv2Parser::DefinitionContext * /*ctx*/) override { }

	void enterDefinition_declaration(SysMLv2Parser::Definition_declarationContext * /*ctx*/) override { }
	void exitDefinition_declaration(SysMLv2Parser::Definition_declarationContext * /*ctx*/) override { }

	void enterDefinition_body(SysMLv2Parser::Definition_bodyContext * /*ctx*/) override { }
	void exitDefinition_body(SysMLv2Parser::Definition_bodyContext * /*ctx*/) override { }

	void enterDefinition_body_item(SysMLv2Parser::Definition_body_itemContext * /*ctx*/) override { }
	void exitDefinition_body_item(SysMLv2Parser::Definition_body_itemContext * /*ctx*/) override { }

	void enterDefinition_member(SysMLv2Parser::Definition_memberContext * /*ctx*/) override { }
	void exitDefinition_member(SysMLv2Parser::Definition_memberContext * /*ctx*/) override { }

	void enterVariant_usage_member(SysMLv2Parser::Variant_usage_memberContext * /*ctx*/) override { }
	void exitVariant_usage_member(SysMLv2Parser::Variant_usage_memberContext * /*ctx*/) override { }

	void enterNon_occurrence_usage_member(SysMLv2Parser::Non_occurrence_usage_memberContext * /*ctx*/) override { }
	void exitNon_occurrence_usage_member(SysMLv2Parser::Non_occurrence_usage_memberContext * /*ctx*/) override { }

	void enterOccurrence_usage_member(SysMLv2Parser::Occurrence_usage_memberContext * /*ctx*/) override { }
	void exitOccurrence_usage_member(SysMLv2Parser::Occurrence_usage_memberContext * /*ctx*/) override { }

	void enterStructure_usage_member(SysMLv2Parser::Structure_usage_memberContext * /*ctx*/) override { }
	void exitStructure_usage_member(SysMLv2Parser::Structure_usage_memberContext * /*ctx*/) override { }

	void enterBehavior_usage_member(SysMLv2Parser::Behavior_usage_memberContext * /*ctx*/) override { }
	void exitBehavior_usage_member(SysMLv2Parser::Behavior_usage_memberContext * /*ctx*/) override { }

	void enterFeature_direction(SysMLv2Parser::Feature_directionContext * /*ctx*/) override { }
	void exitFeature_direction(SysMLv2Parser::Feature_directionContext * /*ctx*/) override { }

	void enterRef_prefix(SysMLv2Parser::Ref_prefixContext * /*ctx*/) override { }
	void exitRef_prefix(SysMLv2Parser::Ref_prefixContext * /*ctx*/) override { }

	void enterBasic_usage_prefix(SysMLv2Parser::Basic_usage_prefixContext * /*ctx*/) override { }
	void exitBasic_usage_prefix(SysMLv2Parser::Basic_usage_prefixContext * /*ctx*/) override { }

	void enterEnd_usage_prefix(SysMLv2Parser::End_usage_prefixContext * /*ctx*/) override { }
	void exitEnd_usage_prefix(SysMLv2Parser::End_usage_prefixContext * /*ctx*/) override { }

	void enterOwned_cross_feature_member(SysMLv2Parser::Owned_cross_feature_memberContext * /*ctx*/) override { }
	void exitOwned_cross_feature_member(SysMLv2Parser::Owned_cross_feature_memberContext * /*ctx*/) override { }

	void enterOwned_cross_feature(SysMLv2Parser::Owned_cross_featureContext * /*ctx*/) override { }
	void exitOwned_cross_feature(SysMLv2Parser::Owned_cross_featureContext * /*ctx*/) override { }

	void enterUsage_extention_keyword(SysMLv2Parser::Usage_extention_keywordContext * /*ctx*/) override { }
	void exitUsage_extention_keyword(SysMLv2Parser::Usage_extention_keywordContext * /*ctx*/) override { }

	void enterUnextended_usage_prefix(SysMLv2Parser::Unextended_usage_prefixContext * /*ctx*/) override { }
	void exitUnextended_usage_prefix(SysMLv2Parser::Unextended_usage_prefixContext * /*ctx*/) override { }

	void enterUsage_prefix(SysMLv2Parser::Usage_prefixContext * /*ctx*/) override { }
	void exitUsage_prefix(SysMLv2Parser::Usage_prefixContext * /*ctx*/) override { }

	void enterUsage(SysMLv2Parser::UsageContext * /*ctx*/) override { }
	void exitUsage(SysMLv2Parser::UsageContext * /*ctx*/) override { }

	void enterUsage_declaration(SysMLv2Parser::Usage_declarationContext * /*ctx*/) override { }
	void exitUsage_declaration(SysMLv2Parser::Usage_declarationContext * /*ctx*/) override { }

	void enterUsage_completion(SysMLv2Parser::Usage_completionContext * /*ctx*/) override { }
	void exitUsage_completion(SysMLv2Parser::Usage_completionContext * /*ctx*/) override { }

	void enterUsage_body(SysMLv2Parser::Usage_bodyContext * /*ctx*/) override { }
	void exitUsage_body(SysMLv2Parser::Usage_bodyContext * /*ctx*/) override { }

	void enterDefault_reference_usage(SysMLv2Parser::Default_reference_usageContext * /*ctx*/) override { }
	void exitDefault_reference_usage(SysMLv2Parser::Default_reference_usageContext * /*ctx*/) override { }

	void enterReference_usage(SysMLv2Parser::Reference_usageContext * /*ctx*/) override { }
	void exitReference_usage(SysMLv2Parser::Reference_usageContext * /*ctx*/) override { }

	void enterVariant_reference(SysMLv2Parser::Variant_referenceContext * /*ctx*/) override { }
	void exitVariant_reference(SysMLv2Parser::Variant_referenceContext * /*ctx*/) override { }

	void enterNon_occurrence_usage_element(SysMLv2Parser::Non_occurrence_usage_elementContext * /*ctx*/) override { }
	void exitNon_occurrence_usage_element(SysMLv2Parser::Non_occurrence_usage_elementContext * /*ctx*/) override { }

	void enterOccurrence_usage_element(SysMLv2Parser::Occurrence_usage_elementContext * /*ctx*/) override { }
	void exitOccurrence_usage_element(SysMLv2Parser::Occurrence_usage_elementContext * /*ctx*/) override { }

	void enterStructure_usage_element(SysMLv2Parser::Structure_usage_elementContext * /*ctx*/) override { }
	void exitStructure_usage_element(SysMLv2Parser::Structure_usage_elementContext * /*ctx*/) override { }

	void enterBehavior_usage_element(SysMLv2Parser::Behavior_usage_elementContext * /*ctx*/) override { }
	void exitBehavior_usage_element(SysMLv2Parser::Behavior_usage_elementContext * /*ctx*/) override { }

	void enterVariant_usage_element(SysMLv2Parser::Variant_usage_elementContext * /*ctx*/) override { }
	void exitVariant_usage_element(SysMLv2Parser::Variant_usage_elementContext * /*ctx*/) override { }

	void enterSubsclassification_part(SysMLv2Parser::Subsclassification_partContext * /*ctx*/) override { }
	void exitSubsclassification_part(SysMLv2Parser::Subsclassification_partContext * /*ctx*/) override { }

	void enterCrosses(SysMLv2Parser::CrossesContext * /*ctx*/) override { }
	void exitCrosses(SysMLv2Parser::CrossesContext * /*ctx*/) override { }

	void enterOwned_cross_subsetting(SysMLv2Parser::Owned_cross_subsettingContext * /*ctx*/) override { }
	void exitOwned_cross_subsetting(SysMLv2Parser::Owned_cross_subsettingContext * /*ctx*/) override { }

	void enterMultiplicity_part(SysMLv2Parser::Multiplicity_partContext * /*ctx*/) override { }
	void exitMultiplicity_part(SysMLv2Parser::Multiplicity_partContext * /*ctx*/) override { }

	void enterOwned_multiplicity(SysMLv2Parser::Owned_multiplicityContext * /*ctx*/) override { }
	void exitOwned_multiplicity(SysMLv2Parser::Owned_multiplicityContext * /*ctx*/) override { }

	void enterMultiplicity_range(SysMLv2Parser::Multiplicity_rangeContext * /*ctx*/) override { }
	void exitMultiplicity_range(SysMLv2Parser::Multiplicity_rangeContext * /*ctx*/) override { }

	void enterAttribute_definition(SysMLv2Parser::Attribute_definitionContext* /*ctx*/) override { }
	void exitAttribute_definition(SysMLv2Parser::Attribute_definitionContext* /*ctx*/) override { }

	void enterAttribute_usage(SysMLv2Parser::Attribute_usageContext* /*ctx*/) override { }
	void exitAttribute_usage(SysMLv2Parser::Attribute_usageContext* ctx) override;

	void enterEnumeration_definition(SysMLv2Parser::Enumeration_definitionContext * /*ctx*/) override { }
	void exitEnumeration_definition(SysMLv2Parser::Enumeration_definitionContext * /*ctx*/) override { }

	void enterEnumeration_body(SysMLv2Parser::Enumeration_bodyContext * /*ctx*/) override { }
	void exitEnumeration_body(SysMLv2Parser::Enumeration_bodyContext * /*ctx*/) override { }

	void enterEnumeration_usage_member(SysMLv2Parser::Enumeration_usage_memberContext * /*ctx*/) override { }
	void exitEnumeration_usage_member(SysMLv2Parser::Enumeration_usage_memberContext * /*ctx*/) override { }

	void enterEnumerated_value(SysMLv2Parser::Enumerated_valueContext * /*ctx*/) override { }
	void exitEnumerated_value(SysMLv2Parser::Enumerated_valueContext * /*ctx*/) override { }

	void enterEnumeration_usage(SysMLv2Parser::Enumeration_usageContext * /*ctx*/) override { }
	void exitEnumeration_usage(SysMLv2Parser::Enumeration_usageContext * /*ctx*/) override { }

	void enterOccurrence_definition_prefix(SysMLv2Parser::Occurrence_definition_prefixContext * /*ctx*/) override { }
	void exitOccurrence_definition_prefix(SysMLv2Parser::Occurrence_definition_prefixContext * /*ctx*/) override { }

	void enterOccurrence_definition(SysMLv2Parser::Occurrence_definitionContext * /*ctx*/) override { }
	void exitOccurrence_definition(SysMLv2Parser::Occurrence_definitionContext * /*ctx*/) override { }

	void enterIndividual_definition(SysMLv2Parser::Individual_definitionContext* ctx) override;
	void exitIndividual_definition(SysMLv2Parser::Individual_definitionContext* ctx) override;

	void enterOccurrence_usage_prefix(SysMLv2Parser::Occurrence_usage_prefixContext * /*ctx*/) override { }
	void exitOccurrence_usage_prefix(SysMLv2Parser::Occurrence_usage_prefixContext * /*ctx*/) override { }

	void enterOccurrence_usage(SysMLv2Parser::Occurrence_usageContext * /*ctx*/) override { }
	void exitOccurrence_usage(SysMLv2Parser::Occurrence_usageContext * /*ctx*/) override { }

	void enterIndividual_usage(SysMLv2Parser::Individual_usageContext* /*ctx*/) override { }
	void exitIndividual_usage(SysMLv2Parser::Individual_usageContext* ctx) override;

	void enterPortion_usage(SysMLv2Parser::Portion_usageContext * /*ctx*/) override { }
	void exitPortion_usage(SysMLv2Parser::Portion_usageContext * /*ctx*/) override { }

	void enterPortion_kind(SysMLv2Parser::Portion_kindContext * /*ctx*/) override { }
	void exitPortion_kind(SysMLv2Parser::Portion_kindContext * /*ctx*/) override { }

	void enterEvent_occurrence_usage(SysMLv2Parser::Event_occurrence_usageContext * /*ctx*/) override { }
	void exitEvent_occurrence_usage(SysMLv2Parser::Event_occurrence_usageContext * /*ctx*/) override { }

	void enterSource_succession_member(SysMLv2Parser::Source_succession_memberContext * /*ctx*/) override { }
	void exitSource_succession_member(SysMLv2Parser::Source_succession_memberContext * /*ctx*/) override { }

	void enterSouce_succession(SysMLv2Parser::Souce_successionContext * /*ctx*/) override { }
	void exitSouce_succession(SysMLv2Parser::Souce_successionContext * /*ctx*/) override { }

	void enterSource_end_member(SysMLv2Parser::Source_end_memberContext * /*ctx*/) override { }
	void exitSource_end_member(SysMLv2Parser::Source_end_memberContext * /*ctx*/) override { }

	void enterSource_end(SysMLv2Parser::Source_endContext * /*ctx*/) override { }
	void exitSource_end(SysMLv2Parser::Source_endContext * /*ctx*/) override { }

	void enterItem_definition(SysMLv2Parser::Item_definitionContext * /*ctx*/) override { }
	void exitItem_definition(SysMLv2Parser::Item_definitionContext * /*ctx*/) override { }

	void enterItem_usage(SysMLv2Parser::Item_usageContext * /*ctx*/) override { }
	void exitItem_usage(SysMLv2Parser::Item_usageContext * /*ctx*/) override { }

	void enterPart_definition(SysMLv2Parser::Part_definitionContext* ctx) override;
	void exitPart_definition(SysMLv2Parser::Part_definitionContext* ctx) override;

	void enterPart_usage(SysMLv2Parser::Part_usageContext* /*ctx*/) override { }
	void exitPart_usage(SysMLv2Parser::Part_usageContext* ctx) override;

	void enterPort_definition(SysMLv2Parser::Port_definitionContext* ctx) override;
	void exitPort_definition(SysMLv2Parser::Port_definitionContext* ctx) override;

	void enterPort_usage(SysMLv2Parser::Port_usageContext* ctx) override;
	void exitPort_usage(SysMLv2Parser::Port_usageContext* ctx) override;

	void enterConjungated_port_typing(SysMLv2Parser::Conjungated_port_typingContext * /*ctx*/) override { }
	void exitConjungated_port_typing(SysMLv2Parser::Conjungated_port_typingContext * /*ctx*/) override { }

	void enterConnection_definition(SysMLv2Parser::Connection_definitionContext * /*ctx*/) override { }
	void exitConnection_definition(SysMLv2Parser::Connection_definitionContext * /*ctx*/) override { }

	void enterConnection_usage(SysMLv2Parser::Connection_usageContext * /*ctx*/) override { }
	void exitConnection_usage(SysMLv2Parser::Connection_usageContext * /*ctx*/) override { }

	void enterConnector_part(SysMLv2Parser::Connector_partContext * /*ctx*/) override { }
	void exitConnector_part(SysMLv2Parser::Connector_partContext * /*ctx*/) override { }

	void enterBinary_connector_part(SysMLv2Parser::Binary_connector_partContext * /*ctx*/) override { }
	void exitBinary_connector_part(SysMLv2Parser::Binary_connector_partContext * /*ctx*/) override { }

	void enterNary_connector_part(SysMLv2Parser::Nary_connector_partContext * /*ctx*/) override { }
	void exitNary_connector_part(SysMLv2Parser::Nary_connector_partContext * /*ctx*/) override { }

	void enterConnector_end_member(SysMLv2Parser::Connector_end_memberContext * /*ctx*/) override { }
	void exitConnector_end_member(SysMLv2Parser::Connector_end_memberContext * /*ctx*/) override { }

	void enterConnecotr_end(SysMLv2Parser::Connecotr_endContext * /*ctx*/) override { }
	void exitConnecotr_end(SysMLv2Parser::Connecotr_endContext * /*ctx*/) override { }

	void enterOwned_cross_multiplicity_member(SysMLv2Parser::Owned_cross_multiplicity_memberContext * /*ctx*/) override { }
	void exitOwned_cross_multiplicity_member(SysMLv2Parser::Owned_cross_multiplicity_memberContext * /*ctx*/) override { }

	void enterOwned_cross_multiplicity(SysMLv2Parser::Owned_cross_multiplicityContext * /*ctx*/) override { }
	void exitOwned_cross_multiplicity(SysMLv2Parser::Owned_cross_multiplicityContext * /*ctx*/) override { }

	void enterBinding_connector_as_usage(SysMLv2Parser::Binding_connector_as_usageContext * /*ctx*/) override { }
	void exitBinding_connector_as_usage(SysMLv2Parser::Binding_connector_as_usageContext* ctx) override;

	void enterSuccession_as_usage(SysMLv2Parser::Succession_as_usageContext * /*ctx*/) override { }
	void exitSuccession_as_usage(SysMLv2Parser::Succession_as_usageContext * /*ctx*/) override { }

	void enterInterface_definition(SysMLv2Parser::Interface_definitionContext * /*ctx*/) override { }
	void exitInterface_definition(SysMLv2Parser::Interface_definitionContext * /*ctx*/) override { }

	void enterInterface_body(SysMLv2Parser::Interface_bodyContext * /*ctx*/) override { }
	void exitInterface_body(SysMLv2Parser::Interface_bodyContext * /*ctx*/) override { }

	void enterInterface_body_item(SysMLv2Parser::Interface_body_itemContext * /*ctx*/) override { }
	void exitInterface_body_item(SysMLv2Parser::Interface_body_itemContext * /*ctx*/) override { }

	void enterInterface_non_occurrence_usage_member(SysMLv2Parser::Interface_non_occurrence_usage_memberContext * /*ctx*/) override { }
	void exitInterface_non_occurrence_usage_member(SysMLv2Parser::Interface_non_occurrence_usage_memberContext * /*ctx*/) override { }

	void enterInterface_non_occurrence_usage_element(SysMLv2Parser::Interface_non_occurrence_usage_elementContext * /*ctx*/) override { }
	void exitInterface_non_occurrence_usage_element(SysMLv2Parser::Interface_non_occurrence_usage_elementContext * /*ctx*/) override { }

	void enterInterface_occurrence_usage_member(SysMLv2Parser::Interface_occurrence_usage_memberContext * /*ctx*/) override { }
	void exitInterface_occurrence_usage_member(SysMLv2Parser::Interface_occurrence_usage_memberContext * /*ctx*/) override { }

	void enterInterface_occurrence_usage_element(SysMLv2Parser::Interface_occurrence_usage_elementContext * /*ctx*/) override { }
	void exitInterface_occurrence_usage_element(SysMLv2Parser::Interface_occurrence_usage_elementContext * /*ctx*/) override { }

	void enterDefault_interface_end(SysMLv2Parser::Default_interface_endContext * /*ctx*/) override { }
	void exitDefault_interface_end(SysMLv2Parser::Default_interface_endContext * /*ctx*/) override { }

	void enterInterface_usage(SysMLv2Parser::Interface_usageContext * /*ctx*/) override { }
	void exitInterface_usage(SysMLv2Parser::Interface_usageContext * /*ctx*/) override { }

	void enterInterface_usage_declaration(SysMLv2Parser::Interface_usage_declarationContext * /*ctx*/) override { }
	void exitInterface_usage_declaration(SysMLv2Parser::Interface_usage_declarationContext * /*ctx*/) override { }

	void enterInterface_part(SysMLv2Parser::Interface_partContext * /*ctx*/) override { }
	void exitInterface_part(SysMLv2Parser::Interface_partContext * /*ctx*/) override { }

	void enterBinary_interface_part(SysMLv2Parser::Binary_interface_partContext * /*ctx*/) override { }
	void exitBinary_interface_part(SysMLv2Parser::Binary_interface_partContext * /*ctx*/) override { }

	void enterNary_interface_part(SysMLv2Parser::Nary_interface_partContext * /*ctx*/) override { }
	void exitNary_interface_part(SysMLv2Parser::Nary_interface_partContext * /*ctx*/) override { }

	void enterInterface_end_member(SysMLv2Parser::Interface_end_memberContext * /*ctx*/) override { }
	void exitInterface_end_member(SysMLv2Parser::Interface_end_memberContext * /*ctx*/) override { }

	void enterInterface_end(SysMLv2Parser::Interface_endContext * /*ctx*/) override { }
	void exitInterface_end(SysMLv2Parser::Interface_endContext * /*ctx*/) override { }

	void enterAllocation_definition(SysMLv2Parser::Allocation_definitionContext * /*ctx*/) override { }
	void exitAllocation_definition(SysMLv2Parser::Allocation_definitionContext * /*ctx*/) override { }

	void enterAllocation_usage(SysMLv2Parser::Allocation_usageContext * /*ctx*/) override { }
	void exitAllocation_usage(SysMLv2Parser::Allocation_usageContext * /*ctx*/) override { }

	void enterAllocation_usage_declaration(SysMLv2Parser::Allocation_usage_declarationContext * /*ctx*/) override { }
	void exitAllocation_usage_declaration(SysMLv2Parser::Allocation_usage_declarationContext * /*ctx*/) override { }

	void enterFlow_definition(SysMLv2Parser::Flow_definitionContext * /*ctx*/) override { }
	void exitFlow_definition(SysMLv2Parser::Flow_definitionContext * /*ctx*/) override { }

	void enterMessage(SysMLv2Parser::MessageContext * /*ctx*/) override { }
	void exitMessage(SysMLv2Parser::MessageContext * /*ctx*/) override { }

	void enterMessage_declaration(SysMLv2Parser::Message_declarationContext * /*ctx*/) override { }
	void exitMessage_declaration(SysMLv2Parser::Message_declarationContext * /*ctx*/) override { }

	void enterMessage_event_member(SysMLv2Parser::Message_event_memberContext * /*ctx*/) override { }
	void exitMessage_event_member(SysMLv2Parser::Message_event_memberContext * /*ctx*/) override { }

	void enterMessage_event(SysMLv2Parser::Message_eventContext * /*ctx*/) override { }
	void exitMessage_event(SysMLv2Parser::Message_eventContext * /*ctx*/) override { }

	void enterFlow_usage(SysMLv2Parser::Flow_usageContext * /*ctx*/) override { }
	void exitFlow_usage(SysMLv2Parser::Flow_usageContext * /*ctx*/) override { }

	void enterSuccession_flow_usage(SysMLv2Parser::Succession_flow_usageContext * /*ctx*/) override { }
	void exitSuccession_flow_usage(SysMLv2Parser::Succession_flow_usageContext * /*ctx*/) override { }

	void enterFlow_declaration(SysMLv2Parser::Flow_declarationContext * /*ctx*/) override { }
	void exitFlow_declaration(SysMLv2Parser::Flow_declarationContext * /*ctx*/) override { }

	void enterFlow_payload_feature_member(SysMLv2Parser::Flow_payload_feature_memberContext * /*ctx*/) override { }
	void exitFlow_payload_feature_member(SysMLv2Parser::Flow_payload_feature_memberContext * /*ctx*/) override { }

	void enterFlow_payload_feature(SysMLv2Parser::Flow_payload_featureContext * /*ctx*/) override { }
	void exitFlow_payload_feature(SysMLv2Parser::Flow_payload_featureContext * /*ctx*/) override { }

	void enterPayload_feature(SysMLv2Parser::Payload_featureContext * /*ctx*/) override { }
	void exitPayload_feature(SysMLv2Parser::Payload_featureContext * /*ctx*/) override { }

	void enterPayload_feature_specialization_part(SysMLv2Parser::Payload_feature_specialization_partContext * /*ctx*/) override { }
	void exitPayload_feature_specialization_part(SysMLv2Parser::Payload_feature_specialization_partContext * /*ctx*/) override { }

	void enterFlow_end_member(SysMLv2Parser::Flow_end_memberContext * /*ctx*/) override { }
	void exitFlow_end_member(SysMLv2Parser::Flow_end_memberContext * /*ctx*/) override { }

	void enterFlow_end(SysMLv2Parser::Flow_endContext * /*ctx*/) override { }
	void exitFlow_end(SysMLv2Parser::Flow_endContext * /*ctx*/) override { }

	void enterFlow_end_subsetting(SysMLv2Parser::Flow_end_subsettingContext * /*ctx*/) override { }
	void exitFlow_end_subsetting(SysMLv2Parser::Flow_end_subsettingContext * /*ctx*/) override { }

	void enterFeature_chain_prefix(SysMLv2Parser::Feature_chain_prefixContext * /*ctx*/) override { }
	void exitFeature_chain_prefix(SysMLv2Parser::Feature_chain_prefixContext * /*ctx*/) override { }

	void enterFlow_feature_member(SysMLv2Parser::Flow_feature_memberContext * /*ctx*/) override { }
	void exitFlow_feature_member(SysMLv2Parser::Flow_feature_memberContext * /*ctx*/) override { }

	void enterFlow_feature(SysMLv2Parser::Flow_featureContext * /*ctx*/) override { }
	void exitFlow_feature(SysMLv2Parser::Flow_featureContext * /*ctx*/) override { }

	void enterFlow_feature_redefinition(SysMLv2Parser::Flow_feature_redefinitionContext * /*ctx*/) override { }
	void exitFlow_feature_redefinition(SysMLv2Parser::Flow_feature_redefinitionContext * /*ctx*/) override { }

	void enterAction_definition(SysMLv2Parser::Action_definitionContext * /*ctx*/) override { }
	void exitAction_definition(SysMLv2Parser::Action_definitionContext * /*ctx*/) override { }

	void enterAction_body(SysMLv2Parser::Action_bodyContext * /*ctx*/) override { }
	void exitAction_body(SysMLv2Parser::Action_bodyContext * /*ctx*/) override { }

	void enterAction_body_item(SysMLv2Parser::Action_body_itemContext * /*ctx*/) override { }
	void exitAction_body_item(SysMLv2Parser::Action_body_itemContext * /*ctx*/) override { }

	void enterNon_behavior_body_item(SysMLv2Parser::Non_behavior_body_itemContext * /*ctx*/) override { }
	void exitNon_behavior_body_item(SysMLv2Parser::Non_behavior_body_itemContext * /*ctx*/) override { }

	void enterAction_behavior_member(SysMLv2Parser::Action_behavior_memberContext * /*ctx*/) override { }
	void exitAction_behavior_member(SysMLv2Parser::Action_behavior_memberContext * /*ctx*/) override { }

	void enterInitial_node_member(SysMLv2Parser::Initial_node_memberContext * /*ctx*/) override { }
	void exitInitial_node_member(SysMLv2Parser::Initial_node_memberContext * /*ctx*/) override { }

	void enterAction_node_member(SysMLv2Parser::Action_node_memberContext * /*ctx*/) override { }
	void exitAction_node_member(SysMLv2Parser::Action_node_memberContext * /*ctx*/) override { }

	void enterAction_target_succession_member(SysMLv2Parser::Action_target_succession_memberContext * /*ctx*/) override { }
	void exitAction_target_succession_member(SysMLv2Parser::Action_target_succession_memberContext * /*ctx*/) override { }

	void enterGuarded_succession_member(SysMLv2Parser::Guarded_succession_memberContext * /*ctx*/) override { }
	void exitGuarded_succession_member(SysMLv2Parser::Guarded_succession_memberContext * /*ctx*/) override { }

	void enterAction_usage(SysMLv2Parser::Action_usageContext * /*ctx*/) override { }
	void exitAction_usage(SysMLv2Parser::Action_usageContext * /*ctx*/) override { }

	void enterAction_usage_declaration(SysMLv2Parser::Action_usage_declarationContext * /*ctx*/) override { }
	void exitAction_usage_declaration(SysMLv2Parser::Action_usage_declarationContext * /*ctx*/) override { }

	void enterPerform_action_usage(SysMLv2Parser::Perform_action_usageContext * /*ctx*/) override { }
	void exitPerform_action_usage(SysMLv2Parser::Perform_action_usageContext * /*ctx*/) override { }

	void enterPerform_action_usage_declaration(SysMLv2Parser::Perform_action_usage_declarationContext * /*ctx*/) override { }
	void exitPerform_action_usage_declaration(SysMLv2Parser::Perform_action_usage_declarationContext * /*ctx*/) override { }

	void enterAction_node(SysMLv2Parser::Action_nodeContext * /*ctx*/) override { }
	void exitAction_node(SysMLv2Parser::Action_nodeContext * /*ctx*/) override { }

	void enterAction_node_usage_declaration(SysMLv2Parser::Action_node_usage_declarationContext * /*ctx*/) override { }
	void exitAction_node_usage_declaration(SysMLv2Parser::Action_node_usage_declarationContext * /*ctx*/) override { }

	void enterAction_node_prefix(SysMLv2Parser::Action_node_prefixContext * /*ctx*/) override { }
	void exitAction_node_prefix(SysMLv2Parser::Action_node_prefixContext * /*ctx*/) override { }

	void enterControl_node(SysMLv2Parser::Control_nodeContext * /*ctx*/) override { }
	void exitControl_node(SysMLv2Parser::Control_nodeContext * /*ctx*/) override { }

	void enterControl_node_prefix(SysMLv2Parser::Control_node_prefixContext * /*ctx*/) override { }
	void exitControl_node_prefix(SysMLv2Parser::Control_node_prefixContext * /*ctx*/) override { }

	void enterMerge_node(SysMLv2Parser::Merge_nodeContext * /*ctx*/) override { }
	void exitMerge_node(SysMLv2Parser::Merge_nodeContext * /*ctx*/) override { }

	void enterDecision_node(SysMLv2Parser::Decision_nodeContext * /*ctx*/) override { }
	void exitDecision_node(SysMLv2Parser::Decision_nodeContext * /*ctx*/) override { }

	void enterJoin_node(SysMLv2Parser::Join_nodeContext * /*ctx*/) override { }
	void exitJoin_node(SysMLv2Parser::Join_nodeContext * /*ctx*/) override { }

	void enterFork_node(SysMLv2Parser::Fork_nodeContext * /*ctx*/) override { }
	void exitFork_node(SysMLv2Parser::Fork_nodeContext * /*ctx*/) override { }

	void enterAccept_node(SysMLv2Parser::Accept_nodeContext * /*ctx*/) override { }
	void exitAccept_node(SysMLv2Parser::Accept_nodeContext * /*ctx*/) override { }

	void enterAccept_node_declaration(SysMLv2Parser::Accept_node_declarationContext * /*ctx*/) override { }
	void exitAccept_node_declaration(SysMLv2Parser::Accept_node_declarationContext * /*ctx*/) override { }

	void enterAccept_parameter_part(SysMLv2Parser::Accept_parameter_partContext * /*ctx*/) override { }
	void exitAccept_parameter_part(SysMLv2Parser::Accept_parameter_partContext * /*ctx*/) override { }

	void enterPayload_parameter_member(SysMLv2Parser::Payload_parameter_memberContext * /*ctx*/) override { }
	void exitPayload_parameter_member(SysMLv2Parser::Payload_parameter_memberContext * /*ctx*/) override { }

	void enterPayload_parameter(SysMLv2Parser::Payload_parameterContext * /*ctx*/) override { }
	void exitPayload_parameter(SysMLv2Parser::Payload_parameterContext * /*ctx*/) override { }

	void enterTrigger_value_part(SysMLv2Parser::Trigger_value_partContext * /*ctx*/) override { }
	void exitTrigger_value_part(SysMLv2Parser::Trigger_value_partContext * /*ctx*/) override { }

	void enterTrigger_expression(SysMLv2Parser::Trigger_expressionContext * /*ctx*/) override { }
	void exitTrigger_expression(SysMLv2Parser::Trigger_expressionContext * /*ctx*/) override { }

	void enterSend_node(SysMLv2Parser::Send_nodeContext * /*ctx*/) override { }
	void exitSend_node(SysMLv2Parser::Send_nodeContext * /*ctx*/) override { }

	void enterSend_node_declaration(SysMLv2Parser::Send_node_declarationContext * /*ctx*/) override { }
	void exitSend_node_declaration(SysMLv2Parser::Send_node_declarationContext * /*ctx*/) override { }

	void enterSender_receiver_part(SysMLv2Parser::Sender_receiver_partContext * /*ctx*/) override { }
	void exitSender_receiver_part(SysMLv2Parser::Sender_receiver_partContext * /*ctx*/) override { }

	void enterNode_parameter_member(SysMLv2Parser::Node_parameter_memberContext * /*ctx*/) override { }
	void exitNode_parameter_member(SysMLv2Parser::Node_parameter_memberContext * /*ctx*/) override { }

	void enterNode_parameter(SysMLv2Parser::Node_parameterContext * /*ctx*/) override { }
	void exitNode_parameter(SysMLv2Parser::Node_parameterContext * /*ctx*/) override { }

	void enterFeature_binding(SysMLv2Parser::Feature_bindingContext * /*ctx*/) override { }
	void exitFeature_binding(SysMLv2Parser::Feature_bindingContext * /*ctx*/) override { }

	void enterAssignment_node(SysMLv2Parser::Assignment_nodeContext * /*ctx*/) override { }
	void exitAssignment_node(SysMLv2Parser::Assignment_nodeContext * /*ctx*/) override { }

	void enterAssignment_node_declaration(SysMLv2Parser::Assignment_node_declarationContext * /*ctx*/) override { }
	void exitAssignment_node_declaration(SysMLv2Parser::Assignment_node_declarationContext * /*ctx*/) override { }

	void enterAssignment_target_member(SysMLv2Parser::Assignment_target_memberContext * /*ctx*/) override { }
	void exitAssignment_target_member(SysMLv2Parser::Assignment_target_memberContext * /*ctx*/) override { }

	void enterAssignment_target_parameter(SysMLv2Parser::Assignment_target_parameterContext * /*ctx*/) override { }
	void exitAssignment_target_parameter(SysMLv2Parser::Assignment_target_parameterContext * /*ctx*/) override { }

	void enterAssignment_target_binding(SysMLv2Parser::Assignment_target_bindingContext * /*ctx*/) override { }
	void exitAssignment_target_binding(SysMLv2Parser::Assignment_target_bindingContext * /*ctx*/) override { }

	void enterFeature_chain_member(SysMLv2Parser::Feature_chain_memberContext * /*ctx*/) override { }
	void exitFeature_chain_member(SysMLv2Parser::Feature_chain_memberContext * /*ctx*/) override { }

	void enterOwned_feature_chain_member(SysMLv2Parser::Owned_feature_chain_memberContext * /*ctx*/) override { }
	void exitOwned_feature_chain_member(SysMLv2Parser::Owned_feature_chain_memberContext * /*ctx*/) override { }

	void enterTerminate_node(SysMLv2Parser::Terminate_nodeContext * /*ctx*/) override { }
	void exitTerminate_node(SysMLv2Parser::Terminate_nodeContext * /*ctx*/) override { }

	void enterIf_node(SysMLv2Parser::If_nodeContext * /*ctx*/) override { }
	void exitIf_node(SysMLv2Parser::If_nodeContext * /*ctx*/) override { }

	void enterExpression_parameter_member(SysMLv2Parser::Expression_parameter_memberContext * /*ctx*/) override { }
	void exitExpression_parameter_member(SysMLv2Parser::Expression_parameter_memberContext * /*ctx*/) override { }

	void enterAction_body_parameter_member(SysMLv2Parser::Action_body_parameter_memberContext * /*ctx*/) override { }
	void exitAction_body_parameter_member(SysMLv2Parser::Action_body_parameter_memberContext * /*ctx*/) override { }

	void enterAction_body_parameter(SysMLv2Parser::Action_body_parameterContext * /*ctx*/) override { }
	void exitAction_body_parameter(SysMLv2Parser::Action_body_parameterContext * /*ctx*/) override { }

	void enterIf_node_parameter_member(SysMLv2Parser::If_node_parameter_memberContext * /*ctx*/) override { }
	void exitIf_node_parameter_member(SysMLv2Parser::If_node_parameter_memberContext * /*ctx*/) override { }

	void enterWhile_loop_node(SysMLv2Parser::While_loop_nodeContext * /*ctx*/) override { }
	void exitWhile_loop_node(SysMLv2Parser::While_loop_nodeContext * /*ctx*/) override { }

	void enterFor_loop_node(SysMLv2Parser::For_loop_nodeContext * /*ctx*/) override { }
	void exitFor_loop_node(SysMLv2Parser::For_loop_nodeContext * /*ctx*/) override { }

	void enterFor_variable_declaration_member(SysMLv2Parser::For_variable_declaration_memberContext * /*ctx*/) override { }
	void exitFor_variable_declaration_member(SysMLv2Parser::For_variable_declaration_memberContext * /*ctx*/) override { }

	void enterFor_variable_declaration(SysMLv2Parser::For_variable_declarationContext * /*ctx*/) override { }
	void exitFor_variable_declaration(SysMLv2Parser::For_variable_declarationContext * /*ctx*/) override { }

	void enterAction_target_succession(SysMLv2Parser::Action_target_successionContext * /*ctx*/) override { }
	void exitAction_target_succession(SysMLv2Parser::Action_target_successionContext * /*ctx*/) override { }

	void enterTarget_succession(SysMLv2Parser::Target_successionContext * /*ctx*/) override { }
	void exitTarget_succession(SysMLv2Parser::Target_successionContext * /*ctx*/) override { }

	void enterGuarded_target_succession(SysMLv2Parser::Guarded_target_successionContext * /*ctx*/) override { }
	void exitGuarded_target_succession(SysMLv2Parser::Guarded_target_successionContext * /*ctx*/) override { }

	void enterDefault_target_succession(SysMLv2Parser::Default_target_successionContext * /*ctx*/) override { }
	void exitDefault_target_succession(SysMLv2Parser::Default_target_successionContext * /*ctx*/) override { }

	void enterGuarded_succession(SysMLv2Parser::Guarded_successionContext * /*ctx*/) override { }
	void exitGuarded_succession(SysMLv2Parser::Guarded_successionContext * /*ctx*/) override { }

	void enterState_definition(SysMLv2Parser::State_definitionContext * /*ctx*/) override { }
	void exitState_definition(SysMLv2Parser::State_definitionContext * /*ctx*/) override { }

	void enterState_def_body(SysMLv2Parser::State_def_bodyContext * /*ctx*/) override { }
	void exitState_def_body(SysMLv2Parser::State_def_bodyContext * /*ctx*/) override { }

	void enterState_body_item(SysMLv2Parser::State_body_itemContext * /*ctx*/) override { }
	void exitState_body_item(SysMLv2Parser::State_body_itemContext * /*ctx*/) override { }

	void enterEntry_action_member(SysMLv2Parser::Entry_action_memberContext * /*ctx*/) override { }
	void exitEntry_action_member(SysMLv2Parser::Entry_action_memberContext * /*ctx*/) override { }

	void enterDo_action_member(SysMLv2Parser::Do_action_memberContext * /*ctx*/) override { }
	void exitDo_action_member(SysMLv2Parser::Do_action_memberContext * /*ctx*/) override { }

	void enterExit_action_member(SysMLv2Parser::Exit_action_memberContext * /*ctx*/) override { }
	void exitExit_action_member(SysMLv2Parser::Exit_action_memberContext * /*ctx*/) override { }

	void enterEntry_transition_member(SysMLv2Parser::Entry_transition_memberContext * /*ctx*/) override { }
	void exitEntry_transition_member(SysMLv2Parser::Entry_transition_memberContext * /*ctx*/) override { }

	void enterState_action_usage(SysMLv2Parser::State_action_usageContext * /*ctx*/) override { }
	void exitState_action_usage(SysMLv2Parser::State_action_usageContext * /*ctx*/) override { }

	void enterEmpty_action_usage(SysMLv2Parser::Empty_action_usageContext * /*ctx*/) override { }
	void exitEmpty_action_usage(SysMLv2Parser::Empty_action_usageContext * /*ctx*/) override { }

	void enterState_perform_action_uage(SysMLv2Parser::State_perform_action_uageContext * /*ctx*/) override { }
	void exitState_perform_action_uage(SysMLv2Parser::State_perform_action_uageContext * /*ctx*/) override { }

	void enterState_accept_action_usage(SysMLv2Parser::State_accept_action_usageContext * /*ctx*/) override { }
	void exitState_accept_action_usage(SysMLv2Parser::State_accept_action_usageContext * /*ctx*/) override { }

	void enterState_send_action_usage(SysMLv2Parser::State_send_action_usageContext * /*ctx*/) override { }
	void exitState_send_action_usage(SysMLv2Parser::State_send_action_usageContext * /*ctx*/) override { }

	void enterState_assignment_action_usage(SysMLv2Parser::State_assignment_action_usageContext * /*ctx*/) override { }
	void exitState_assignment_action_usage(SysMLv2Parser::State_assignment_action_usageContext * /*ctx*/) override { }

	void enterTransition_usage_member(SysMLv2Parser::Transition_usage_memberContext * /*ctx*/) override { }
	void exitTransition_usage_member(SysMLv2Parser::Transition_usage_memberContext * /*ctx*/) override { }

	void enterTarget_transition_usage_member(SysMLv2Parser::Target_transition_usage_memberContext * /*ctx*/) override { }
	void exitTarget_transition_usage_member(SysMLv2Parser::Target_transition_usage_memberContext * /*ctx*/) override { }

	void enterState_usage(SysMLv2Parser::State_usageContext * /*ctx*/) override { }
	void exitState_usage(SysMLv2Parser::State_usageContext * /*ctx*/) override { }

	void enterState_usage_body(SysMLv2Parser::State_usage_bodyContext * /*ctx*/) override { }
	void exitState_usage_body(SysMLv2Parser::State_usage_bodyContext * /*ctx*/) override { }

	void enterExhibit_state_usage(SysMLv2Parser::Exhibit_state_usageContext * /*ctx*/) override { }
	void exitExhibit_state_usage(SysMLv2Parser::Exhibit_state_usageContext * /*ctx*/) override { }

	void enterTransition_usage(SysMLv2Parser::Transition_usageContext * /*ctx*/) override { }
	void exitTransition_usage(SysMLv2Parser::Transition_usageContext * /*ctx*/) override { }

	void enterTarget_transition_usage(SysMLv2Parser::Target_transition_usageContext * /*ctx*/) override { }
	void exitTarget_transition_usage(SysMLv2Parser::Target_transition_usageContext * /*ctx*/) override { }

	void enterTrigger_action_member(SysMLv2Parser::Trigger_action_memberContext * /*ctx*/) override { }
	void exitTrigger_action_member(SysMLv2Parser::Trigger_action_memberContext * /*ctx*/) override { }

	void enterTrigger_action(SysMLv2Parser::Trigger_actionContext * /*ctx*/) override { }
	void exitTrigger_action(SysMLv2Parser::Trigger_actionContext * /*ctx*/) override { }

	void enterGuard_expression_member(SysMLv2Parser::Guard_expression_memberContext * /*ctx*/) override { }
	void exitGuard_expression_member(SysMLv2Parser::Guard_expression_memberContext * /*ctx*/) override { }

	void enterEffect_behavior_member(SysMLv2Parser::Effect_behavior_memberContext * /*ctx*/) override { }
	void exitEffect_behavior_member(SysMLv2Parser::Effect_behavior_memberContext * /*ctx*/) override { }

	void enterEffect_behavior_usage(SysMLv2Parser::Effect_behavior_usageContext * /*ctx*/) override { }
	void exitEffect_behavior_usage(SysMLv2Parser::Effect_behavior_usageContext * /*ctx*/) override { }

	void enterTransition_perform_action_usage(SysMLv2Parser::Transition_perform_action_usageContext * /*ctx*/) override { }
	void exitTransition_perform_action_usage(SysMLv2Parser::Transition_perform_action_usageContext * /*ctx*/) override { }

	void enterTransition_accept_action_usage(SysMLv2Parser::Transition_accept_action_usageContext * /*ctx*/) override { }
	void exitTransition_accept_action_usage(SysMLv2Parser::Transition_accept_action_usageContext * /*ctx*/) override { }

	void enterTransition_send_action_usage(SysMLv2Parser::Transition_send_action_usageContext * /*ctx*/) override { }
	void exitTransition_send_action_usage(SysMLv2Parser::Transition_send_action_usageContext * /*ctx*/) override { }

	void enterTransition_assignment_action_usage(SysMLv2Parser::Transition_assignment_action_usageContext * /*ctx*/) override { }
	void exitTransition_assignment_action_usage(SysMLv2Parser::Transition_assignment_action_usageContext * /*ctx*/) override { }

	void enterTransition_succession_member(SysMLv2Parser::Transition_succession_memberContext * /*ctx*/) override { }
	void exitTransition_succession_member(SysMLv2Parser::Transition_succession_memberContext * /*ctx*/) override { }

	void enterTransition_succession(SysMLv2Parser::Transition_successionContext * /*ctx*/) override { }
	void exitTransition_succession(SysMLv2Parser::Transition_successionContext * /*ctx*/) override { }

	void enterCalculation_definition(SysMLv2Parser::Calculation_definitionContext * /*ctx*/) override { }
	void exitCalculation_definition(SysMLv2Parser::Calculation_definitionContext * /*ctx*/) override { }

	void enterCalculation_usage(SysMLv2Parser::Calculation_usageContext * /*ctx*/) override { }
	void exitCalculation_usage(SysMLv2Parser::Calculation_usageContext * /*ctx*/) override { }

	void enterCalculation_body(SysMLv2Parser::Calculation_bodyContext * /*ctx*/) override { }
	void exitCalculation_body(SysMLv2Parser::Calculation_bodyContext * /*ctx*/) override { }

	void enterCalculation_body_part(SysMLv2Parser::Calculation_body_partContext * /*ctx*/) override { }
	void exitCalculation_body_part(SysMLv2Parser::Calculation_body_partContext * /*ctx*/) override { }

	void enterCalculation_body_item(SysMLv2Parser::Calculation_body_itemContext * /*ctx*/) override { }
	void exitCalculation_body_item(SysMLv2Parser::Calculation_body_itemContext * /*ctx*/) override { }

	void enterReturn_parameter_member(SysMLv2Parser::Return_parameter_memberContext * /*ctx*/) override { }
	void exitReturn_parameter_member(SysMLv2Parser::Return_parameter_memberContext * /*ctx*/) override { }

	void enterResult_expression_member(SysMLv2Parser::Result_expression_memberContext * /*ctx*/) override { }
	void exitResult_expression_member(SysMLv2Parser::Result_expression_memberContext * /*ctx*/) override { }

	void enterConstraint_definition(SysMLv2Parser::Constraint_definitionContext * /*ctx*/) override { }
	void exitConstraint_definition(SysMLv2Parser::Constraint_definitionContext * /*ctx*/) override { }

	void enterConstraint_usage(SysMLv2Parser::Constraint_usageContext * /*ctx*/) override { }
	void exitConstraint_usage(SysMLv2Parser::Constraint_usageContext * /*ctx*/) override { }

	void enterAssert_constriant_usage(SysMLv2Parser::Assert_constriant_usageContext * /*ctx*/) override { }
	void exitAssert_constriant_usage(SysMLv2Parser::Assert_constriant_usageContext * /*ctx*/) override { }

	void enterConstraint_usage_declaration(SysMLv2Parser::Constraint_usage_declarationContext * /*ctx*/) override { }
	void exitConstraint_usage_declaration(SysMLv2Parser::Constraint_usage_declarationContext * /*ctx*/) override { }

	void enterRequirement_definition(SysMLv2Parser::Requirement_definitionContext * /*ctx*/) override { }
	void exitRequirement_definition(SysMLv2Parser::Requirement_definitionContext * /*ctx*/) override { }

	void enterRequirement_body(SysMLv2Parser::Requirement_bodyContext * /*ctx*/) override { }
	void exitRequirement_body(SysMLv2Parser::Requirement_bodyContext * /*ctx*/) override { }

	void enterRequirement_body_item(SysMLv2Parser::Requirement_body_itemContext * /*ctx*/) override { }
	void exitRequirement_body_item(SysMLv2Parser::Requirement_body_itemContext * /*ctx*/) override { }

	void enterSubject_member(SysMLv2Parser::Subject_memberContext * /*ctx*/) override { }
	void exitSubject_member(SysMLv2Parser::Subject_memberContext * /*ctx*/) override { }

	void enterSubject_usage(SysMLv2Parser::Subject_usageContext * /*ctx*/) override { }
	void exitSubject_usage(SysMLv2Parser::Subject_usageContext * /*ctx*/) override { }

	void enterRequirement_constraint_member(SysMLv2Parser::Requirement_constraint_memberContext * /*ctx*/) override { }
	void exitRequirement_constraint_member(SysMLv2Parser::Requirement_constraint_memberContext * /*ctx*/) override { }

	void enterRequriement_kind(SysMLv2Parser::Requriement_kindContext * /*ctx*/) override { }
	void exitRequriement_kind(SysMLv2Parser::Requriement_kindContext * /*ctx*/) override { }

	void enterRequirement_constraint_usage(SysMLv2Parser::Requirement_constraint_usageContext * /*ctx*/) override { }
	void exitRequirement_constraint_usage(SysMLv2Parser::Requirement_constraint_usageContext * /*ctx*/) override { }

	void enterFramed_concern_member(SysMLv2Parser::Framed_concern_memberContext * /*ctx*/) override { }
	void exitFramed_concern_member(SysMLv2Parser::Framed_concern_memberContext * /*ctx*/) override { }

	void enterFramed_concern_usage(SysMLv2Parser::Framed_concern_usageContext * /*ctx*/) override { }
	void exitFramed_concern_usage(SysMLv2Parser::Framed_concern_usageContext * /*ctx*/) override { }

	void enterActor_member(SysMLv2Parser::Actor_memberContext * /*ctx*/) override { }
	void exitActor_member(SysMLv2Parser::Actor_memberContext * /*ctx*/) override { }

	void enterActor_usage(SysMLv2Parser::Actor_usageContext * /*ctx*/) override { }
	void exitActor_usage(SysMLv2Parser::Actor_usageContext * /*ctx*/) override { }

	void enterStakeholder_member(SysMLv2Parser::Stakeholder_memberContext * /*ctx*/) override { }
	void exitStakeholder_member(SysMLv2Parser::Stakeholder_memberContext * /*ctx*/) override { }

	void enterStakeholder_usage(SysMLv2Parser::Stakeholder_usageContext * /*ctx*/) override { }
	void exitStakeholder_usage(SysMLv2Parser::Stakeholder_usageContext * /*ctx*/) override { }

	void enterRequirement_usage(SysMLv2Parser::Requirement_usageContext * /*ctx*/) override { }
	void exitRequirement_usage(SysMLv2Parser::Requirement_usageContext * /*ctx*/) override { }

	void enterSatisfy_requirement_usage(SysMLv2Parser::Satisfy_requirement_usageContext * /*ctx*/) override { }
	void exitSatisfy_requirement_usage(SysMLv2Parser::Satisfy_requirement_usageContext * /*ctx*/) override { }

	void enterSatisfaction_subject_member(SysMLv2Parser::Satisfaction_subject_memberContext * /*ctx*/) override { }
	void exitSatisfaction_subject_member(SysMLv2Parser::Satisfaction_subject_memberContext * /*ctx*/) override { }

	void enterSatisfaction_parameter(SysMLv2Parser::Satisfaction_parameterContext * /*ctx*/) override { }
	void exitSatisfaction_parameter(SysMLv2Parser::Satisfaction_parameterContext * /*ctx*/) override { }

	void enterSatisfaction_feature_value(SysMLv2Parser::Satisfaction_feature_valueContext * /*ctx*/) override { }
	void exitSatisfaction_feature_value(SysMLv2Parser::Satisfaction_feature_valueContext * /*ctx*/) override { }

	void enterSatisfaction_reference_expression(SysMLv2Parser::Satisfaction_reference_expressionContext * /*ctx*/) override { }
	void exitSatisfaction_reference_expression(SysMLv2Parser::Satisfaction_reference_expressionContext * /*ctx*/) override { }

	void enterConcern_definition(SysMLv2Parser::Concern_definitionContext * /*ctx*/) override { }
	void exitConcern_definition(SysMLv2Parser::Concern_definitionContext * /*ctx*/) override { }

	void enterConcern_usage(SysMLv2Parser::Concern_usageContext * /*ctx*/) override { }
	void exitConcern_usage(SysMLv2Parser::Concern_usageContext * /*ctx*/) override { }

	void enterCase_definition(SysMLv2Parser::Case_definitionContext * /*ctx*/) override { }
	void exitCase_definition(SysMLv2Parser::Case_definitionContext * /*ctx*/) override { }

	void enterCase_usage(SysMLv2Parser::Case_usageContext * /*ctx*/) override { }
	void exitCase_usage(SysMLv2Parser::Case_usageContext * /*ctx*/) override { }

	void enterCase_body(SysMLv2Parser::Case_bodyContext * /*ctx*/) override { }
	void exitCase_body(SysMLv2Parser::Case_bodyContext * /*ctx*/) override { }

	void enterCase_body_item(SysMLv2Parser::Case_body_itemContext * /*ctx*/) override { }
	void exitCase_body_item(SysMLv2Parser::Case_body_itemContext * /*ctx*/) override { }

	void enterObjective_member(SysMLv2Parser::Objective_memberContext * /*ctx*/) override { }
	void exitObjective_member(SysMLv2Parser::Objective_memberContext * /*ctx*/) override { }

	void enterObjective_requirement_usage(SysMLv2Parser::Objective_requirement_usageContext * /*ctx*/) override { }
	void exitObjective_requirement_usage(SysMLv2Parser::Objective_requirement_usageContext * /*ctx*/) override { }

	void enterAnalysis_case_definition(SysMLv2Parser::Analysis_case_definitionContext * /*ctx*/) override { }
	void exitAnalysis_case_definition(SysMLv2Parser::Analysis_case_definitionContext * /*ctx*/) override { }

	void enterAnalysis_case_usage(SysMLv2Parser::Analysis_case_usageContext * /*ctx*/) override { }
	void exitAnalysis_case_usage(SysMLv2Parser::Analysis_case_usageContext * /*ctx*/) override { }

	void enterVerification_case_definition(SysMLv2Parser::Verification_case_definitionContext * /*ctx*/) override { }
	void exitVerification_case_definition(SysMLv2Parser::Verification_case_definitionContext * /*ctx*/) override { }

	void enterVerification_case_usage(SysMLv2Parser::Verification_case_usageContext * /*ctx*/) override { }
	void exitVerification_case_usage(SysMLv2Parser::Verification_case_usageContext * /*ctx*/) override { }

	void enterRequirement_verification_member(SysMLv2Parser::Requirement_verification_memberContext * /*ctx*/) override { }
	void exitRequirement_verification_member(SysMLv2Parser::Requirement_verification_memberContext * /*ctx*/) override { }

	void enterRequirement_verification_usage(SysMLv2Parser::Requirement_verification_usageContext * /*ctx*/) override { }
	void exitRequirement_verification_usage(SysMLv2Parser::Requirement_verification_usageContext * /*ctx*/) override { }

	void enterUse_case_definition(SysMLv2Parser::Use_case_definitionContext * /*ctx*/) override { }
	void exitUse_case_definition(SysMLv2Parser::Use_case_definitionContext * /*ctx*/) override { }

	void enterUse_case_usage(SysMLv2Parser::Use_case_usageContext * /*ctx*/) override { }
	void exitUse_case_usage(SysMLv2Parser::Use_case_usageContext * /*ctx*/) override { }

	void enterInclude_use_case_usage(SysMLv2Parser::Include_use_case_usageContext * /*ctx*/) override { }
	void exitInclude_use_case_usage(SysMLv2Parser::Include_use_case_usageContext * /*ctx*/) override { }

	void enterView_definition(SysMLv2Parser::View_definitionContext * /*ctx*/) override { }
	void exitView_definition(SysMLv2Parser::View_definitionContext * /*ctx*/) override { }

	void enterView_definition_body(SysMLv2Parser::View_definition_bodyContext * /*ctx*/) override { }
	void exitView_definition_body(SysMLv2Parser::View_definition_bodyContext * /*ctx*/) override { }

	void enterView_definition_body_item(SysMLv2Parser::View_definition_body_itemContext * /*ctx*/) override { }
	void exitView_definition_body_item(SysMLv2Parser::View_definition_body_itemContext * /*ctx*/) override { }

	void enterView_rendering_member(SysMLv2Parser::View_rendering_memberContext * /*ctx*/) override { }
	void exitView_rendering_member(SysMLv2Parser::View_rendering_memberContext * /*ctx*/) override { }

	void enterView_rendering_usage(SysMLv2Parser::View_rendering_usageContext * /*ctx*/) override { }
	void exitView_rendering_usage(SysMLv2Parser::View_rendering_usageContext * /*ctx*/) override { }

	void enterView_usage(SysMLv2Parser::View_usageContext * /*ctx*/) override { }
	void exitView_usage(SysMLv2Parser::View_usageContext * /*ctx*/) override { }

	void enterView_body(SysMLv2Parser::View_bodyContext * /*ctx*/) override { }
	void exitView_body(SysMLv2Parser::View_bodyContext * /*ctx*/) override { }

	void enterView_body_item(SysMLv2Parser::View_body_itemContext * /*ctx*/) override { }
	void exitView_body_item(SysMLv2Parser::View_body_itemContext * /*ctx*/) override { }

	void enterExpose(SysMLv2Parser::ExposeContext * /*ctx*/) override { }
	void exitExpose(SysMLv2Parser::ExposeContext * /*ctx*/) override { }

	void enterMembership_expose(SysMLv2Parser::Membership_exposeContext * /*ctx*/) override { }
	void exitMembership_expose(SysMLv2Parser::Membership_exposeContext * /*ctx*/) override { }

	void enterNamespace_expose(SysMLv2Parser::Namespace_exposeContext * /*ctx*/) override { }
	void exitNamespace_expose(SysMLv2Parser::Namespace_exposeContext * /*ctx*/) override { }

	void enterViewpoint_definition(SysMLv2Parser::Viewpoint_definitionContext * /*ctx*/) override { }
	void exitViewpoint_definition(SysMLv2Parser::Viewpoint_definitionContext * /*ctx*/) override { }

	void enterViewpoint_usage(SysMLv2Parser::Viewpoint_usageContext * /*ctx*/) override { }
	void exitViewpoint_usage(SysMLv2Parser::Viewpoint_usageContext * /*ctx*/) override { }

	void enterRendering_definition(SysMLv2Parser::Rendering_definitionContext * /*ctx*/) override { }
	void exitRendering_definition(SysMLv2Parser::Rendering_definitionContext * /*ctx*/) override { }

	void enterRendering_usage(SysMLv2Parser::Rendering_usageContext * /*ctx*/) override { }
	void exitRendering_usage(SysMLv2Parser::Rendering_usageContext * /*ctx*/) override { }

	void enterMetadata_definition(SysMLv2Parser::Metadata_definitionContext * /*ctx*/) override { }
	void exitMetadata_definition(SysMLv2Parser::Metadata_definitionContext * /*ctx*/) override { }

	void enterPrefix_metadata_annotation(SysMLv2Parser::Prefix_metadata_annotationContext * /*ctx*/) override { }
	void exitPrefix_metadata_annotation(SysMLv2Parser::Prefix_metadata_annotationContext * /*ctx*/) override { }

	void enterPrefix_metadata_member(SysMLv2Parser::Prefix_metadata_memberContext * /*ctx*/) override { }
	void exitPrefix_metadata_member(SysMLv2Parser::Prefix_metadata_memberContext * /*ctx*/) override { }

	void enterPrefix_metadata_usage(SysMLv2Parser::Prefix_metadata_usageContext * /*ctx*/) override { }
	void exitPrefix_metadata_usage(SysMLv2Parser::Prefix_metadata_usageContext * /*ctx*/) override { }

	void enterMetadata_usage(SysMLv2Parser::Metadata_usageContext * /*ctx*/) override { }
	void exitMetadata_usage(SysMLv2Parser::Metadata_usageContext * /*ctx*/) override { }

	void enterMetadata_usage_declaration(SysMLv2Parser::Metadata_usage_declarationContext * /*ctx*/) override { }
	void exitMetadata_usage_declaration(SysMLv2Parser::Metadata_usage_declarationContext * /*ctx*/) override { }

	void enterMetadata_body(SysMLv2Parser::Metadata_bodyContext * /*ctx*/) override { }
	void exitMetadata_body(SysMLv2Parser::Metadata_bodyContext * /*ctx*/) override { }

	void enterMetadata_body_usage_member(SysMLv2Parser::Metadata_body_usage_memberContext * /*ctx*/) override { }
	void exitMetadata_body_usage_member(SysMLv2Parser::Metadata_body_usage_memberContext * /*ctx*/) override { }

	void enterMetadata_body_usage(SysMLv2Parser::Metadata_body_usageContext * /*ctx*/) override { }
	void exitMetadata_body_usage(SysMLv2Parser::Metadata_body_usageContext * /*ctx*/) override { }

	void enterExtended_definition(SysMLv2Parser::Extended_definitionContext * /*ctx*/) override { }
	void exitExtended_definition(SysMLv2Parser::Extended_definitionContext * /*ctx*/) override { }

	void enterExtended_usage(SysMLv2Parser::Extended_usageContext * /*ctx*/) override { }
	void exitExtended_usage(SysMLv2Parser::Extended_usageContext * /*ctx*/) override { }

	void enterIdentification(SysMLv2Parser::IdentificationContext * /*ctx*/) override { }
	void exitIdentification(SysMLv2Parser::IdentificationContext * /*ctx*/) override { }

	void enterRelationship_onwed_elements(SysMLv2Parser::Relationship_onwed_elementsContext * /*ctx*/) override { }
	void exitRelationship_onwed_elements(SysMLv2Parser::Relationship_onwed_elementsContext * /*ctx*/) override { }

	void enterRelationship_owned_element(SysMLv2Parser::Relationship_owned_elementContext * /*ctx*/) override { }
	void exitRelationship_owned_element(SysMLv2Parser::Relationship_owned_elementContext * /*ctx*/) override { }

	void enterOwned_related_element(SysMLv2Parser::Owned_related_elementContext * /*ctx*/) override { }
	void exitOwned_related_element(SysMLv2Parser::Owned_related_elementContext * /*ctx*/) override { }

	void enterNamespace(SysMLv2Parser::NamespaceContext * /*ctx*/) override { }
	void exitNamespace(SysMLv2Parser::NamespaceContext * /*ctx*/) override { }

	void enterNamespace_declaration(SysMLv2Parser::Namespace_declarationContext * /*ctx*/) override { }
	void exitNamespace_declaration(SysMLv2Parser::Namespace_declarationContext * /*ctx*/) override { }

	void enterNamespace_body(SysMLv2Parser::Namespace_bodyContext * /*ctx*/) override { }
	void exitNamespace_body(SysMLv2Parser::Namespace_bodyContext * /*ctx*/) override { }

	void enterNamespace_body_elements(SysMLv2Parser::Namespace_body_elementsContext * /*ctx*/) override { }
	void exitNamespace_body_elements(SysMLv2Parser::Namespace_body_elementsContext * /*ctx*/) override { }

	void enterNamespace_body_element(SysMLv2Parser::Namespace_body_elementContext * /*ctx*/) override { }
	void exitNamespace_body_element(SysMLv2Parser::Namespace_body_elementContext * /*ctx*/) override { }

	void enterMember_prefix(SysMLv2Parser::Member_prefixContext * /*ctx*/) override { }
	void exitMember_prefix(SysMLv2Parser::Member_prefixContext * /*ctx*/) override { }

	void enterVisibility_indicator(SysMLv2Parser::Visibility_indicatorContext * /*ctx*/) override { }
	void exitVisibility_indicator(SysMLv2Parser::Visibility_indicatorContext * /*ctx*/) override { }

	void enterNamespace_member(SysMLv2Parser::Namespace_memberContext * /*ctx*/) override { }
	void exitNamespace_member(SysMLv2Parser::Namespace_memberContext * /*ctx*/) override { }

	void enterNon_feature_member(SysMLv2Parser::Non_feature_memberContext * /*ctx*/) override { }
	void exitNon_feature_member(SysMLv2Parser::Non_feature_memberContext * /*ctx*/) override { }

	void enterNamespace_feature_member(SysMLv2Parser::Namespace_feature_memberContext * /*ctx*/) override { }
	void exitNamespace_feature_member(SysMLv2Parser::Namespace_feature_memberContext * /*ctx*/) override { }

	void enterQualified_name(SysMLv2Parser::Qualified_nameContext * /*ctx*/) override { }
	void exitQualified_name(SysMLv2Parser::Qualified_nameContext * /*ctx*/) override { }

	void enterNamespace_import(SysMLv2Parser::Namespace_importContext * /*ctx*/) override { }
	void exitNamespace_import(SysMLv2Parser::Namespace_importContext * /*ctx*/) override { }

	void enterImport_declaration(SysMLv2Parser::Import_declarationContext * /*ctx*/) override { }
	void exitImport_declaration(SysMLv2Parser::Import_declarationContext * /*ctx*/) override { }

	void enterMembership_import(SysMLv2Parser::Membership_importContext * /*ctx*/) override { }
	void exitMembership_import(SysMLv2Parser::Membership_importContext * /*ctx*/) override { }

	void enterFilter_package(SysMLv2Parser::Filter_packageContext * /*ctx*/) override { }
	void exitFilter_package(SysMLv2Parser::Filter_packageContext * /*ctx*/) override { }

	void enterFilter_package_member(SysMLv2Parser::Filter_package_memberContext * /*ctx*/) override { }
	void exitFilter_package_member(SysMLv2Parser::Filter_package_memberContext * /*ctx*/) override { }

	void enterElement(SysMLv2Parser::ElementContext * /*ctx*/) override { }
	void exitElement(SysMLv2Parser::ElementContext * /*ctx*/) override { }

	void enterNon_feature_element(SysMLv2Parser::Non_feature_elementContext * /*ctx*/) override { }
	void exitNon_feature_element(SysMLv2Parser::Non_feature_elementContext * /*ctx*/) override { }

	void enterFeature_element(SysMLv2Parser::Feature_elementContext * /*ctx*/) override { }
	void exitFeature_element(SysMLv2Parser::Feature_elementContext * /*ctx*/) override { }

	void enterAdditional_options(SysMLv2Parser::Additional_optionsContext * /*ctx*/) override { }
	void exitAdditional_options(SysMLv2Parser::Additional_optionsContext * /*ctx*/) override { }

	void enterType(SysMLv2Parser::TypeContext * /*ctx*/) override { }
	void exitType(SysMLv2Parser::TypeContext * /*ctx*/) override { }

	void enterType_prefix(SysMLv2Parser::Type_prefixContext * /*ctx*/) override { }
	void exitType_prefix(SysMLv2Parser::Type_prefixContext * /*ctx*/) override { }

	void enterType_declaration(SysMLv2Parser::Type_declarationContext * /*ctx*/) override { }
	void exitType_declaration(SysMLv2Parser::Type_declarationContext * /*ctx*/) override { }

	void enterSpecialization_part(SysMLv2Parser::Specialization_partContext * /*ctx*/) override { }
	void exitSpecialization_part(SysMLv2Parser::Specialization_partContext * /*ctx*/) override { }

	void enterConjugation_part(SysMLv2Parser::Conjugation_partContext * /*ctx*/) override { }
	void exitConjugation_part(SysMLv2Parser::Conjugation_partContext * /*ctx*/) override { }

	void enterType_relationship_part(SysMLv2Parser::Type_relationship_partContext * /*ctx*/) override { }
	void exitType_relationship_part(SysMLv2Parser::Type_relationship_partContext * /*ctx*/) override { }

	void enterDisjoining_part(SysMLv2Parser::Disjoining_partContext * /*ctx*/) override { }
	void exitDisjoining_part(SysMLv2Parser::Disjoining_partContext * /*ctx*/) override { }

	void enterUnioning_part(SysMLv2Parser::Unioning_partContext * /*ctx*/) override { }
	void exitUnioning_part(SysMLv2Parser::Unioning_partContext * /*ctx*/) override { }

	void enterIntersecting_part(SysMLv2Parser::Intersecting_partContext * /*ctx*/) override { }
	void exitIntersecting_part(SysMLv2Parser::Intersecting_partContext * /*ctx*/) override { }

	void enterDifferencing_part(SysMLv2Parser::Differencing_partContext * /*ctx*/) override { }
	void exitDifferencing_part(SysMLv2Parser::Differencing_partContext * /*ctx*/) override { }

	void enterType_body(SysMLv2Parser::Type_bodyContext * /*ctx*/) override { }
	void exitType_body(SysMLv2Parser::Type_bodyContext * /*ctx*/) override { }

	void enterType_body_elements(SysMLv2Parser::Type_body_elementsContext * /*ctx*/) override { }
	void exitType_body_elements(SysMLv2Parser::Type_body_elementsContext * /*ctx*/) override { }

	void enterType_body_element(SysMLv2Parser::Type_body_elementContext * /*ctx*/) override { }
	void exitType_body_element(SysMLv2Parser::Type_body_elementContext * /*ctx*/) override { }

	void enterSpecialization(SysMLv2Parser::SpecializationContext * /*ctx*/) override { }
	void exitSpecialization(SysMLv2Parser::SpecializationContext * /*ctx*/) override { }

	void enterOwned_specialization(SysMLv2Parser::Owned_specializationContext * /*ctx*/) override { }
	void exitOwned_specialization(SysMLv2Parser::Owned_specializationContext * /*ctx*/) override { }

	void enterSpecific_type(SysMLv2Parser::Specific_typeContext * /*ctx*/) override { }
	void exitSpecific_type(SysMLv2Parser::Specific_typeContext * /*ctx*/) override { }

	void enterGeneral_type(SysMLv2Parser::General_typeContext * /*ctx*/) override { }
	void exitGeneral_type(SysMLv2Parser::General_typeContext * /*ctx*/) override { }

	void enterConjunction(SysMLv2Parser::ConjunctionContext * /*ctx*/) override { }
	void exitConjunction(SysMLv2Parser::ConjunctionContext * /*ctx*/) override { }

	void enterOwned_conjugation(SysMLv2Parser::Owned_conjugationContext * /*ctx*/) override { }
	void exitOwned_conjugation(SysMLv2Parser::Owned_conjugationContext * /*ctx*/) override { }

	void enterDisjoining(SysMLv2Parser::DisjoiningContext * /*ctx*/) override { }
	void exitDisjoining(SysMLv2Parser::DisjoiningContext * /*ctx*/) override { }

	void enterOwned_disjoining(SysMLv2Parser::Owned_disjoiningContext * /*ctx*/) override { }
	void exitOwned_disjoining(SysMLv2Parser::Owned_disjoiningContext * /*ctx*/) override { }

	void enterUnioning(SysMLv2Parser::UnioningContext * /*ctx*/) override { }
	void exitUnioning(SysMLv2Parser::UnioningContext * /*ctx*/) override { }

	void enterIntersecting(SysMLv2Parser::IntersectingContext * /*ctx*/) override { }
	void exitIntersecting(SysMLv2Parser::IntersectingContext * /*ctx*/) override { }

	void enterDifferencing(SysMLv2Parser::DifferencingContext * /*ctx*/) override { }
	void exitDifferencing(SysMLv2Parser::DifferencingContext * /*ctx*/) override { }

	void enterFeature_member(SysMLv2Parser::Feature_memberContext * /*ctx*/) override { }
	void exitFeature_member(SysMLv2Parser::Feature_memberContext * /*ctx*/) override { }

	void enterType_feature_member(SysMLv2Parser::Type_feature_memberContext * /*ctx*/) override { }
	void exitType_feature_member(SysMLv2Parser::Type_feature_memberContext * /*ctx*/) override { }

	void enterOwned_feature_member(SysMLv2Parser::Owned_feature_memberContext * /*ctx*/) override { }
	void exitOwned_feature_member(SysMLv2Parser::Owned_feature_memberContext * /*ctx*/) override { }

	void enterClassifier(SysMLv2Parser::ClassifierContext * /*ctx*/) override { }
	void exitClassifier(SysMLv2Parser::ClassifierContext * /*ctx*/) override { }

	void enterClassifier_declaration(SysMLv2Parser::Classifier_declarationContext * /*ctx*/) override { }
	void exitClassifier_declaration(SysMLv2Parser::Classifier_declarationContext * /*ctx*/) override { }

	void enterSuperclassing_part(SysMLv2Parser::Superclassing_partContext * /*ctx*/) override { }
	void exitSuperclassing_part(SysMLv2Parser::Superclassing_partContext * /*ctx*/) override { }

	void enterSubclassification(SysMLv2Parser::SubclassificationContext * /*ctx*/) override { }
	void exitSubclassification(SysMLv2Parser::SubclassificationContext * /*ctx*/) override { }

	void enterOwned_subclassification(SysMLv2Parser::Owned_subclassificationContext * /*ctx*/) override { }
	void exitOwned_subclassification(SysMLv2Parser::Owned_subclassificationContext * /*ctx*/) override { }

	void enterFeature(SysMLv2Parser::FeatureContext * /*ctx*/) override { }
	void exitFeature(SysMLv2Parser::FeatureContext * /*ctx*/) override { }

	void enterFeature_prefix(SysMLv2Parser::Feature_prefixContext * /*ctx*/) override { }
	void exitFeature_prefix(SysMLv2Parser::Feature_prefixContext * /*ctx*/) override { }

	void enterFeature_declaration(SysMLv2Parser::Feature_declarationContext * /*ctx*/) override { }
	void exitFeature_declaration(SysMLv2Parser::Feature_declarationContext * /*ctx*/) override { }

	void enterFeature_identification(SysMLv2Parser::Feature_identificationContext * /*ctx*/) override { }
	void exitFeature_identification(SysMLv2Parser::Feature_identificationContext * /*ctx*/) override { }

	void enterFeature_relationship_part(SysMLv2Parser::Feature_relationship_partContext * /*ctx*/) override { }
	void exitFeature_relationship_part(SysMLv2Parser::Feature_relationship_partContext * /*ctx*/) override { }

	void enterChaining_part(SysMLv2Parser::Chaining_partContext * /*ctx*/) override { }
	void exitChaining_part(SysMLv2Parser::Chaining_partContext * /*ctx*/) override { }

	void enterInverting_part(SysMLv2Parser::Inverting_partContext * /*ctx*/) override { }
	void exitInverting_part(SysMLv2Parser::Inverting_partContext * /*ctx*/) override { }

	void enterType_featuring_part(SysMLv2Parser::Type_featuring_partContext * /*ctx*/) override { }
	void exitType_featuring_part(SysMLv2Parser::Type_featuring_partContext * /*ctx*/) override { }

	void enterFeature_specialization_part(SysMLv2Parser::Feature_specialization_partContext * /*ctx*/) override { }
	void exitFeature_specialization_part(SysMLv2Parser::Feature_specialization_partContext * /*ctx*/) override { }

	void enterFeature_specilization(SysMLv2Parser::Feature_specilizationContext * /*ctx*/) override { }
	void exitFeature_specilization(SysMLv2Parser::Feature_specilizationContext * /*ctx*/) override { }

	void enterTypings(SysMLv2Parser::TypingsContext * /*ctx*/) override { }
	void exitTypings(SysMLv2Parser::TypingsContext * /*ctx*/) override { }

	void enterOwned_typed_by(SysMLv2Parser::Owned_typed_byContext * /*ctx*/) override { }
	void exitOwned_typed_by(SysMLv2Parser::Owned_typed_byContext * /*ctx*/) override { }

	void enterSubsettings(SysMLv2Parser::SubsettingsContext * /*ctx*/) override { }
	void exitSubsettings(SysMLv2Parser::SubsettingsContext * /*ctx*/) override { }

	void enterSubsets(SysMLv2Parser::SubsetsContext * /*ctx*/) override { }
	void exitSubsets(SysMLv2Parser::SubsetsContext * /*ctx*/) override { }

	void enterReferences(SysMLv2Parser::ReferencesContext * /*ctx*/) override { }
	void exitReferences(SysMLv2Parser::ReferencesContext * /*ctx*/) override { }

	void enterRedefinitions(SysMLv2Parser::RedefinitionsContext * /*ctx*/) override { }
	void exitRedefinitions(SysMLv2Parser::RedefinitionsContext * /*ctx*/) override { }

	void enterRedefines(SysMLv2Parser::RedefinesContext * /*ctx*/) override { }
	void exitRedefines(SysMLv2Parser::RedefinesContext * /*ctx*/) override { }

	void enterFeature_typing(SysMLv2Parser::Feature_typingContext * /*ctx*/) override { }
	void exitFeature_typing(SysMLv2Parser::Feature_typingContext * /*ctx*/) override { }

	void enterOwned_feature_typing(SysMLv2Parser::Owned_feature_typingContext * /*ctx*/) override { }
	void exitOwned_feature_typing(SysMLv2Parser::Owned_feature_typingContext * /*ctx*/) override { }

	void enterSubsetting(SysMLv2Parser::SubsettingContext * /*ctx*/) override { }
	void exitSubsetting(SysMLv2Parser::SubsettingContext * /*ctx*/) override { }

	void enterOwned_subsetting(SysMLv2Parser::Owned_subsettingContext * /*ctx*/) override { }
	void exitOwned_subsetting(SysMLv2Parser::Owned_subsettingContext * /*ctx*/) override { }

	void enterOwned_reference_subsetting(SysMLv2Parser::Owned_reference_subsettingContext * /*ctx*/) override { }
	void exitOwned_reference_subsetting(SysMLv2Parser::Owned_reference_subsettingContext * /*ctx*/) override { }

	void enterRedefinition(SysMLv2Parser::RedefinitionContext * /*ctx*/) override { }
	void exitRedefinition(SysMLv2Parser::RedefinitionContext * /*ctx*/) override { }

	void enterOwned_redefinition(SysMLv2Parser::Owned_redefinitionContext * /*ctx*/) override { }
	void exitOwned_redefinition(SysMLv2Parser::Owned_redefinitionContext * /*ctx*/) override { }

	void enterOwned_feature_chain(SysMLv2Parser::Owned_feature_chainContext * /*ctx*/) override { }
	void exitOwned_feature_chain(SysMLv2Parser::Owned_feature_chainContext * /*ctx*/) override { }

	void enterFeature_chain(SysMLv2Parser::Feature_chainContext * /*ctx*/) override { }
	void exitFeature_chain(SysMLv2Parser::Feature_chainContext * /*ctx*/) override { }

	void enterOwned_feature_chaining(SysMLv2Parser::Owned_feature_chainingContext * /*ctx*/) override { }
	void exitOwned_feature_chaining(SysMLv2Parser::Owned_feature_chainingContext * /*ctx*/) override { }

	void enterFeature_inverting(SysMLv2Parser::Feature_invertingContext * /*ctx*/) override { }
	void exitFeature_inverting(SysMLv2Parser::Feature_invertingContext * /*ctx*/) override { }

	void enterOwned_feature_inverting(SysMLv2Parser::Owned_feature_invertingContext * /*ctx*/) override { }
	void exitOwned_feature_inverting(SysMLv2Parser::Owned_feature_invertingContext * /*ctx*/) override { }

	void enterType_featuring(SysMLv2Parser::Type_featuringContext * /*ctx*/) override { }
	void exitType_featuring(SysMLv2Parser::Type_featuringContext * /*ctx*/) override { }

	void enterOwned_type_featuring(SysMLv2Parser::Owned_type_featuringContext * /*ctx*/) override { }
	void exitOwned_type_featuring(SysMLv2Parser::Owned_type_featuringContext * /*ctx*/) override { }

	void enterData_type(SysMLv2Parser::Data_typeContext * /*ctx*/) override { }
	void exitData_type(SysMLv2Parser::Data_typeContext * /*ctx*/) override { }

	void enterClass(SysMLv2Parser::ClassContext * /*ctx*/) override { }
	void exitClass(SysMLv2Parser::ClassContext * /*ctx*/) override { }

	void enterStructure(SysMLv2Parser::StructureContext * /*ctx*/) override { }
	void exitStructure(SysMLv2Parser::StructureContext * /*ctx*/) override { }

	void enterAssociation(SysMLv2Parser::AssociationContext * /*ctx*/) override { }
	void exitAssociation(SysMLv2Parser::AssociationContext * /*ctx*/) override { }

	void enterAssociation_structure(SysMLv2Parser::Association_structureContext * /*ctx*/) override { }
	void exitAssociation_structure(SysMLv2Parser::Association_structureContext * /*ctx*/) override { }

	void enterConnector(SysMLv2Parser::ConnectorContext * /*ctx*/) override { }
	void exitConnector(SysMLv2Parser::ConnectorContext * /*ctx*/) override { }

	void enterConnector_declaration(SysMLv2Parser::Connector_declarationContext * /*ctx*/) override { }
	void exitConnector_declaration(SysMLv2Parser::Connector_declarationContext * /*ctx*/) override { }

	void enterBinary_connector_declaration(SysMLv2Parser::Binary_connector_declarationContext * /*ctx*/) override { }
	void exitBinary_connector_declaration(SysMLv2Parser::Binary_connector_declarationContext * /*ctx*/) override { }

	void enterNary_connector_declaration(SysMLv2Parser::Nary_connector_declarationContext * /*ctx*/) override { }
	void exitNary_connector_declaration(SysMLv2Parser::Nary_connector_declarationContext * /*ctx*/) override { }

	void enterConnector_end(SysMLv2Parser::Connector_endContext * /*ctx*/) override { }
	void exitConnector_end(SysMLv2Parser::Connector_endContext * /*ctx*/) override { }

	void enterBinding_connector(SysMLv2Parser::Binding_connectorContext * /*ctx*/) override { }
	void exitBinding_connector(SysMLv2Parser::Binding_connectorContext * /*ctx*/) override { }

	void enterBinding_connector_declaration(SysMLv2Parser::Binding_connector_declarationContext * /*ctx*/) override { }
	void exitBinding_connector_declaration(SysMLv2Parser::Binding_connector_declarationContext * /*ctx*/) override { }

	void enterSuccession(SysMLv2Parser::SuccessionContext * /*ctx*/) override { }
	void exitSuccession(SysMLv2Parser::SuccessionContext * /*ctx*/) override { }

	void enterSuccession_declaration(SysMLv2Parser::Succession_declarationContext * /*ctx*/) override { }
	void exitSuccession_declaration(SysMLv2Parser::Succession_declarationContext * /*ctx*/) override { }

	void enterBehavior(SysMLv2Parser::BehaviorContext * /*ctx*/) override { }
	void exitBehavior(SysMLv2Parser::BehaviorContext * /*ctx*/) override { }

	void enterStep(SysMLv2Parser::StepContext * /*ctx*/) override { }
	void exitStep(SysMLv2Parser::StepContext * /*ctx*/) override { }

	void enterFunction(SysMLv2Parser::FunctionContext * /*ctx*/) override { }
	void exitFunction(SysMLv2Parser::FunctionContext * /*ctx*/) override { }

	void enterFunction_body(SysMLv2Parser::Function_bodyContext * /*ctx*/) override { }
	void exitFunction_body(SysMLv2Parser::Function_bodyContext * /*ctx*/) override { }

	void enterFunction_body_part(SysMLv2Parser::Function_body_partContext * /*ctx*/) override { }
	void exitFunction_body_part(SysMLv2Parser::Function_body_partContext * /*ctx*/) override { }

	void enterReturn_feature_member(SysMLv2Parser::Return_feature_memberContext * /*ctx*/) override { }
	void exitReturn_feature_member(SysMLv2Parser::Return_feature_memberContext * /*ctx*/) override { }

	void enterExpression(SysMLv2Parser::ExpressionContext * /*ctx*/) override { }
	void exitExpression(SysMLv2Parser::ExpressionContext * /*ctx*/) override { }

	void enterPredicate(SysMLv2Parser::PredicateContext * /*ctx*/) override { }
	void exitPredicate(SysMLv2Parser::PredicateContext * /*ctx*/) override { }

	void enterBoolean_expression(SysMLv2Parser::Boolean_expressionContext * /*ctx*/) override { }
	void exitBoolean_expression(SysMLv2Parser::Boolean_expressionContext * /*ctx*/) override { }

	void enterInvariant(SysMLv2Parser::InvariantContext * /*ctx*/) override { }
	void exitInvariant(SysMLv2Parser::InvariantContext * /*ctx*/) override { }

	void enterOwned_expression_reference_member(SysMLv2Parser::Owned_expression_reference_memberContext * /*ctx*/) override { }
	void exitOwned_expression_reference_member(SysMLv2Parser::Owned_expression_reference_memberContext * /*ctx*/) override { }

	void enterOwned_expression_reference(SysMLv2Parser::Owned_expression_referenceContext * /*ctx*/) override { }
	void exitOwned_expression_reference(SysMLv2Parser::Owned_expression_referenceContext * /*ctx*/) override { }

	void enterOwned_expression_member(SysMLv2Parser::Owned_expression_memberContext * /*ctx*/) override { }
	void exitOwned_expression_member(SysMLv2Parser::Owned_expression_memberContext * /*ctx*/) override { }

	void enterOwned_expressions(SysMLv2Parser::Owned_expressionsContext * /*ctx*/) override { }
	void exitOwned_expressions(SysMLv2Parser::Owned_expressionsContext * /*ctx*/) override { }

	void enterOwned_expression(SysMLv2Parser::Owned_expressionContext * /*ctx*/) override { }
	void exitOwned_expression(SysMLv2Parser::Owned_expressionContext * /*ctx*/) override { }

	void enterEased_owned_expression(SysMLv2Parser::Eased_owned_expressionContext * /*ctx*/) override { }
	void exitEased_owned_expression(SysMLv2Parser::Eased_owned_expressionContext * /*ctx*/) override { }

	void enterConditional_expression(SysMLv2Parser::Conditional_expressionContext * /*ctx*/) override { }
	void exitConditional_expression(SysMLv2Parser::Conditional_expressionContext * /*ctx*/) override { }

	void enterConditional_binary_operator_expression(SysMLv2Parser::Conditional_binary_operator_expressionContext * /*ctx*/) override { }
	void exitConditional_binary_operator_expression(SysMLv2Parser::Conditional_binary_operator_expressionContext * /*ctx*/) override { }

	void enterConditional_binary_operator(SysMLv2Parser::Conditional_binary_operatorContext * /*ctx*/) override { }
	void exitConditional_binary_operator(SysMLv2Parser::Conditional_binary_operatorContext * /*ctx*/) override { }

	void enterBinary_operator_expression(SysMLv2Parser::Binary_operator_expressionContext * /*ctx*/) override { }
	void exitBinary_operator_expression(SysMLv2Parser::Binary_operator_expressionContext * /*ctx*/) override { }

	void enterBinary_operator(SysMLv2Parser::Binary_operatorContext * /*ctx*/) override { }
	void exitBinary_operator(SysMLv2Parser::Binary_operatorContext * /*ctx*/) override { }

	void enterUnary_operator_expression(SysMLv2Parser::Unary_operator_expressionContext * /*ctx*/) override { }
	void exitUnary_operator_expression(SysMLv2Parser::Unary_operator_expressionContext * /*ctx*/) override { }

	void enterUnary_operator(SysMLv2Parser::Unary_operatorContext * /*ctx*/) override { }
	void exitUnary_operator(SysMLv2Parser::Unary_operatorContext * /*ctx*/) override { }

	void enterClassification_expression(SysMLv2Parser::Classification_expressionContext * /*ctx*/) override { }
	void exitClassification_expression(SysMLv2Parser::Classification_expressionContext * /*ctx*/) override { }

	void enterClassification(SysMLv2Parser::ClassificationContext * /*ctx*/) override { }
	void exitClassification(SysMLv2Parser::ClassificationContext * /*ctx*/) override { }

	void enterClassification_test_operator(SysMLv2Parser::Classification_test_operatorContext * /*ctx*/) override { }
	void exitClassification_test_operator(SysMLv2Parser::Classification_test_operatorContext * /*ctx*/) override { }

	void enterCast_operator(SysMLv2Parser::Cast_operatorContext * /*ctx*/) override { }
	void exitCast_operator(SysMLv2Parser::Cast_operatorContext * /*ctx*/) override { }

	void enterMetaclassification_expression(SysMLv2Parser::Metaclassification_expressionContext * /*ctx*/) override { }
	void exitMetaclassification_expression(SysMLv2Parser::Metaclassification_expressionContext * /*ctx*/) override { }

	void enterArgument_member(SysMLv2Parser::Argument_memberContext * /*ctx*/) override { }
	void exitArgument_member(SysMLv2Parser::Argument_memberContext * /*ctx*/) override { }

	void enterArgument(SysMLv2Parser::ArgumentContext * /*ctx*/) override { }
	void exitArgument(SysMLv2Parser::ArgumentContext * /*ctx*/) override { }

	void enterArgument_value(SysMLv2Parser::Argument_valueContext * /*ctx*/) override { }
	void exitArgument_value(SysMLv2Parser::Argument_valueContext * /*ctx*/) override { }

	void enterArgument_expression_member(SysMLv2Parser::Argument_expression_memberContext * /*ctx*/) override { }
	void exitArgument_expression_member(SysMLv2Parser::Argument_expression_memberContext * /*ctx*/) override { }

	void enterArgument_expression(SysMLv2Parser::Argument_expressionContext * /*ctx*/) override { }
	void exitArgument_expression(SysMLv2Parser::Argument_expressionContext * /*ctx*/) override { }

	void enterArgument_expression_value(SysMLv2Parser::Argument_expression_valueContext * /*ctx*/) override { }
	void exitArgument_expression_value(SysMLv2Parser::Argument_expression_valueContext * /*ctx*/) override { }

	void enterMetadata_argument_member(SysMLv2Parser::Metadata_argument_memberContext * /*ctx*/) override { }
	void exitMetadata_argument_member(SysMLv2Parser::Metadata_argument_memberContext * /*ctx*/) override { }

	void enterMetadata_argument(SysMLv2Parser::Metadata_argumentContext * /*ctx*/) override { }
	void exitMetadata_argument(SysMLv2Parser::Metadata_argumentContext * /*ctx*/) override { }

	void enterMetadata_value(SysMLv2Parser::Metadata_valueContext * /*ctx*/) override { }
	void exitMetadata_value(SysMLv2Parser::Metadata_valueContext * /*ctx*/) override { }

	void enterMetadata_reference(SysMLv2Parser::Metadata_referenceContext * /*ctx*/) override { }
	void exitMetadata_reference(SysMLv2Parser::Metadata_referenceContext * /*ctx*/) override { }

	void enterMetadataclassification_test_operator(SysMLv2Parser::Metadataclassification_test_operatorContext * /*ctx*/) override { }
	void exitMetadataclassification_test_operator(SysMLv2Parser::Metadataclassification_test_operatorContext * /*ctx*/) override { }

	void enterMeta_cast_operator(SysMLv2Parser::Meta_cast_operatorContext * /*ctx*/) override { }
	void exitMeta_cast_operator(SysMLv2Parser::Meta_cast_operatorContext * /*ctx*/) override { }

	void enterExtend_expression(SysMLv2Parser::Extend_expressionContext * /*ctx*/) override { }
	void exitExtend_expression(SysMLv2Parser::Extend_expressionContext * /*ctx*/) override { }

	void enterType_reference_member(SysMLv2Parser::Type_reference_memberContext * /*ctx*/) override { }
	void exitType_reference_member(SysMLv2Parser::Type_reference_memberContext * /*ctx*/) override { }

	void enterType_result_member(SysMLv2Parser::Type_result_memberContext * /*ctx*/) override { }
	void exitType_result_member(SysMLv2Parser::Type_result_memberContext * /*ctx*/) override { }

	void enterType_reference(SysMLv2Parser::Type_referenceContext * /*ctx*/) override { }
	void exitType_reference(SysMLv2Parser::Type_referenceContext * /*ctx*/) override { }

	void enterReference_typing(SysMLv2Parser::Reference_typingContext * /*ctx*/) override { }
	void exitReference_typing(SysMLv2Parser::Reference_typingContext * /*ctx*/) override { }

	void enterPrimary_expressions(SysMLv2Parser::Primary_expressionsContext * /*ctx*/) override { }
	void exitPrimary_expressions(SysMLv2Parser::Primary_expressionsContext * /*ctx*/) override { }

	void enterPrimary_expression(SysMLv2Parser::Primary_expressionContext * /*ctx*/) override { }
	void exitPrimary_expression(SysMLv2Parser::Primary_expressionContext * /*ctx*/) override { }

	void enterPrimary_argument_value(SysMLv2Parser::Primary_argument_valueContext * /*ctx*/) override { }
	void exitPrimary_argument_value(SysMLv2Parser::Primary_argument_valueContext * /*ctx*/) override { }

	void enterPrimary_argument(SysMLv2Parser::Primary_argumentContext * /*ctx*/) override { }
	void exitPrimary_argument(SysMLv2Parser::Primary_argumentContext * /*ctx*/) override { }

	void enterPrimary_argument_member(SysMLv2Parser::Primary_argument_memberContext * /*ctx*/) override { }
	void exitPrimary_argument_member(SysMLv2Parser::Primary_argument_memberContext * /*ctx*/) override { }

	void enterNon_feature_chain_primary_expression(SysMLv2Parser::Non_feature_chain_primary_expressionContext * /*ctx*/) override { }
	void exitNon_feature_chain_primary_expression(SysMLv2Parser::Non_feature_chain_primary_expressionContext * /*ctx*/) override { }

	void enterNon_feature_chain_primary_argument_value(SysMLv2Parser::Non_feature_chain_primary_argument_valueContext * /*ctx*/) override { }
	void exitNon_feature_chain_primary_argument_value(SysMLv2Parser::Non_feature_chain_primary_argument_valueContext * /*ctx*/) override { }

	void enterNon_feature_chain_primary_argument(SysMLv2Parser::Non_feature_chain_primary_argumentContext * /*ctx*/) override { }
	void exitNon_feature_chain_primary_argument(SysMLv2Parser::Non_feature_chain_primary_argumentContext * /*ctx*/) override { }

	void enterNon_feature_chain_primary_argument_member(SysMLv2Parser::Non_feature_chain_primary_argument_memberContext * /*ctx*/) override { }
	void exitNon_feature_chain_primary_argument_member(SysMLv2Parser::Non_feature_chain_primary_argument_memberContext * /*ctx*/) override { }

	void enterBracket_expression(SysMLv2Parser::Bracket_expressionContext * /*ctx*/) override { }
	void exitBracket_expression(SysMLv2Parser::Bracket_expressionContext * /*ctx*/) override { }

	void enterIndex_expression(SysMLv2Parser::Index_expressionContext * /*ctx*/) override { }
	void exitIndex_expression(SysMLv2Parser::Index_expressionContext * /*ctx*/) override { }

	void enterSequence_expression(SysMLv2Parser::Sequence_expressionContext * /*ctx*/) override { }
	void exitSequence_expression(SysMLv2Parser::Sequence_expressionContext * /*ctx*/) override { }

	void enterSequence_expression_list(SysMLv2Parser::Sequence_expression_listContext * /*ctx*/) override { }
	void exitSequence_expression_list(SysMLv2Parser::Sequence_expression_listContext * /*ctx*/) override { }

	void enterSequence_operator_expression(SysMLv2Parser::Sequence_operator_expressionContext * /*ctx*/) override { }
	void exitSequence_operator_expression(SysMLv2Parser::Sequence_operator_expressionContext * /*ctx*/) override { }

	void enterSequence_expression_list_member(SysMLv2Parser::Sequence_expression_list_memberContext * /*ctx*/) override { }
	void exitSequence_expression_list_member(SysMLv2Parser::Sequence_expression_list_memberContext * /*ctx*/) override { }

	void enterFeature_chain_expression(SysMLv2Parser::Feature_chain_expressionContext * /*ctx*/) override { }
	void exitFeature_chain_expression(SysMLv2Parser::Feature_chain_expressionContext * /*ctx*/) override { }

	void enterCollect_expression(SysMLv2Parser::Collect_expressionContext * /*ctx*/) override { }
	void exitCollect_expression(SysMLv2Parser::Collect_expressionContext * /*ctx*/) override { }

	void enterSelect_expression(SysMLv2Parser::Select_expressionContext * /*ctx*/) override { }
	void exitSelect_expression(SysMLv2Parser::Select_expressionContext * /*ctx*/) override { }

	void enterFunction_operation_expression(SysMLv2Parser::Function_operation_expressionContext * /*ctx*/) override { }
	void exitFunction_operation_expression(SysMLv2Parser::Function_operation_expressionContext * /*ctx*/) override { }

	void enterBody_argument_member(SysMLv2Parser::Body_argument_memberContext * /*ctx*/) override { }
	void exitBody_argument_member(SysMLv2Parser::Body_argument_memberContext * /*ctx*/) override { }

	void enterBody_argument(SysMLv2Parser::Body_argumentContext * /*ctx*/) override { }
	void exitBody_argument(SysMLv2Parser::Body_argumentContext * /*ctx*/) override { }

	void enterBody_argument_value(SysMLv2Parser::Body_argument_valueContext * /*ctx*/) override { }
	void exitBody_argument_value(SysMLv2Parser::Body_argument_valueContext * /*ctx*/) override { }

	void enterBody_expression_member(SysMLv2Parser::Body_expression_memberContext * /*ctx*/) override { }
	void exitBody_expression_member(SysMLv2Parser::Body_expression_memberContext * /*ctx*/) override { }

	void enterFunction_reference_argument_member(SysMLv2Parser::Function_reference_argument_memberContext * /*ctx*/) override { }
	void exitFunction_reference_argument_member(SysMLv2Parser::Function_reference_argument_memberContext * /*ctx*/) override { }

	void enterFunction_reference_argument(SysMLv2Parser::Function_reference_argumentContext * /*ctx*/) override { }
	void exitFunction_reference_argument(SysMLv2Parser::Function_reference_argumentContext * /*ctx*/) override { }

	void enterFunction_reference_arugment_value(SysMLv2Parser::Function_reference_arugment_valueContext * /*ctx*/) override { }
	void exitFunction_reference_arugment_value(SysMLv2Parser::Function_reference_arugment_valueContext * /*ctx*/) override { }

	void enterFunction_reference_expression(SysMLv2Parser::Function_reference_expressionContext * /*ctx*/) override { }
	void exitFunction_reference_expression(SysMLv2Parser::Function_reference_expressionContext * /*ctx*/) override { }

	void enterFunction_reference_member(SysMLv2Parser::Function_reference_memberContext * /*ctx*/) override { }
	void exitFunction_reference_member(SysMLv2Parser::Function_reference_memberContext * /*ctx*/) override { }

	void enterFunction_reference(SysMLv2Parser::Function_referenceContext * /*ctx*/) override { }
	void exitFunction_reference(SysMLv2Parser::Function_referenceContext * /*ctx*/) override { }

	void enterBase_expression(SysMLv2Parser::Base_expressionContext * /*ctx*/) override { }
	void exitBase_expression(SysMLv2Parser::Base_expressionContext * /*ctx*/) override { }

	void enterNull_expression(SysMLv2Parser::Null_expressionContext * /*ctx*/) override { }
	void exitNull_expression(SysMLv2Parser::Null_expressionContext * /*ctx*/) override { }

	void enterFeature_reference_expression(SysMLv2Parser::Feature_reference_expressionContext * /*ctx*/) override { }
	void exitFeature_reference_expression(SysMLv2Parser::Feature_reference_expressionContext * /*ctx*/) override { }

	void enterFeature_reference_member(SysMLv2Parser::Feature_reference_memberContext * /*ctx*/) override { }
	void exitFeature_reference_member(SysMLv2Parser::Feature_reference_memberContext * /*ctx*/) override { }

	void enterFeature_reference(SysMLv2Parser::Feature_referenceContext * /*ctx*/) override { }
	void exitFeature_reference(SysMLv2Parser::Feature_referenceContext * /*ctx*/) override { }

	void enterMetadata_access_expression(SysMLv2Parser::Metadata_access_expressionContext * /*ctx*/) override { }
	void exitMetadata_access_expression(SysMLv2Parser::Metadata_access_expressionContext * /*ctx*/) override { }

	void enterInvocation_expression(SysMLv2Parser::Invocation_expressionContext * /*ctx*/) override { }
	void exitInvocation_expression(SysMLv2Parser::Invocation_expressionContext * /*ctx*/) override { }

	void enterInternal_invocation_expression(SysMLv2Parser::Internal_invocation_expressionContext * /*ctx*/) override { }
	void exitInternal_invocation_expression(SysMLv2Parser::Internal_invocation_expressionContext * /*ctx*/) override { }

	void enterArgument_list(SysMLv2Parser::Argument_listContext * /*ctx*/) override { }
	void exitArgument_list(SysMLv2Parser::Argument_listContext * /*ctx*/) override { }

	void enterPositional_argument_list(SysMLv2Parser::Positional_argument_listContext * /*ctx*/) override { }
	void exitPositional_argument_list(SysMLv2Parser::Positional_argument_listContext * /*ctx*/) override { }

	void enterNamed_argument_list(SysMLv2Parser::Named_argument_listContext * /*ctx*/) override { }
	void exitNamed_argument_list(SysMLv2Parser::Named_argument_listContext * /*ctx*/) override { }

	void enterNamed_argument_member(SysMLv2Parser::Named_argument_memberContext * /*ctx*/) override { }
	void exitNamed_argument_member(SysMLv2Parser::Named_argument_memberContext * /*ctx*/) override { }

	void enterNamed_argument(SysMLv2Parser::Named_argumentContext * /*ctx*/) override { }
	void exitNamed_argument(SysMLv2Parser::Named_argumentContext * /*ctx*/) override { }

	void enterParamenter_redefinition(SysMLv2Parser::Paramenter_redefinitionContext * /*ctx*/) override { }
	void exitParamenter_redefinition(SysMLv2Parser::Paramenter_redefinitionContext * /*ctx*/) override { }

	void enterBody_expression(SysMLv2Parser::Body_expressionContext * /*ctx*/) override { }
	void exitBody_expression(SysMLv2Parser::Body_expressionContext * /*ctx*/) override { }

	void enterExpression_body_member(SysMLv2Parser::Expression_body_memberContext * /*ctx*/) override { }
	void exitExpression_body_member(SysMLv2Parser::Expression_body_memberContext * /*ctx*/) override { }

	void enterExpression_body(SysMLv2Parser::Expression_bodyContext * /*ctx*/) override { }
	void exitExpression_body(SysMLv2Parser::Expression_bodyContext * /*ctx*/) override { }

	void enterLiteral_expression(SysMLv2Parser::Literal_expressionContext * /*ctx*/) override { }
	void exitLiteral_expression(SysMLv2Parser::Literal_expressionContext * /*ctx*/) override { }

	void enterLiteral_boolean(SysMLv2Parser::Literal_booleanContext * /*ctx*/) override { }
	void exitLiteral_boolean(SysMLv2Parser::Literal_booleanContext * /*ctx*/) override { }

	void enterBoolean_value(SysMLv2Parser::Boolean_valueContext * /*ctx*/) override { }
	void exitBoolean_value(SysMLv2Parser::Boolean_valueContext * /*ctx*/) override { }

	void enterLiteral_string(SysMLv2Parser::Literal_stringContext * /*ctx*/) override { }
	void exitLiteral_string(SysMLv2Parser::Literal_stringContext * /*ctx*/) override { }

	void enterLiteral_integer(SysMLv2Parser::Literal_integerContext * /*ctx*/) override { }
	void exitLiteral_integer(SysMLv2Parser::Literal_integerContext * /*ctx*/) override { }

	void enterLiteral_real(SysMLv2Parser::Literal_realContext * /*ctx*/) override { }
	void exitLiteral_real(SysMLv2Parser::Literal_realContext * /*ctx*/) override { }

	void enterReal_value(SysMLv2Parser::Real_valueContext * /*ctx*/) override { }
	void exitReal_value(SysMLv2Parser::Real_valueContext * /*ctx*/) override { }

	void enterLiteral_infinity(SysMLv2Parser::Literal_infinityContext * /*ctx*/) override { }
	void exitLiteral_infinity(SysMLv2Parser::Literal_infinityContext * /*ctx*/) override { }

	void enterInteraction(SysMLv2Parser::InteractionContext * /*ctx*/) override { }
	void exitInteraction(SysMLv2Parser::InteractionContext * /*ctx*/) override { }

	void enterItem_flow(SysMLv2Parser::Item_flowContext * /*ctx*/) override { }
	void exitItem_flow(SysMLv2Parser::Item_flowContext * /*ctx*/) override { }

	void enterSuccession_item_flow(SysMLv2Parser::Succession_item_flowContext * /*ctx*/) override { }
	void exitSuccession_item_flow(SysMLv2Parser::Succession_item_flowContext * /*ctx*/) override { }

	void enterItem_flow_declaration(SysMLv2Parser::Item_flow_declarationContext * /*ctx*/) override { }
	void exitItem_flow_declaration(SysMLv2Parser::Item_flow_declarationContext * /*ctx*/) override { }

	void enterItem_feature_member(SysMLv2Parser::Item_feature_memberContext * /*ctx*/) override { }
	void exitItem_feature_member(SysMLv2Parser::Item_feature_memberContext * /*ctx*/) override { }

	void enterItem_feature(SysMLv2Parser::Item_featureContext * /*ctx*/) override { }
	void exitItem_feature(SysMLv2Parser::Item_featureContext * /*ctx*/) override { }

	void enterItem_feature_specilization_part(SysMLv2Parser::Item_feature_specilization_partContext * /*ctx*/) override { }
	void exitItem_feature_specilization_part(SysMLv2Parser::Item_feature_specilization_partContext * /*ctx*/) override { }

	void enterItem_flow_end_member(SysMLv2Parser::Item_flow_end_memberContext * /*ctx*/) override { }
	void exitItem_flow_end_member(SysMLv2Parser::Item_flow_end_memberContext * /*ctx*/) override { }

	void enterItem_flow_end(SysMLv2Parser::Item_flow_endContext * /*ctx*/) override { }
	void exitItem_flow_end(SysMLv2Parser::Item_flow_endContext * /*ctx*/) override { }

	void enterItem_flow_feature_member(SysMLv2Parser::Item_flow_feature_memberContext * /*ctx*/) override { }
	void exitItem_flow_feature_member(SysMLv2Parser::Item_flow_feature_memberContext * /*ctx*/) override { }

	void enterItem_flow_feature(SysMLv2Parser::Item_flow_featureContext * /*ctx*/) override { }
	void exitItem_flow_feature(SysMLv2Parser::Item_flow_featureContext * /*ctx*/) override { }

	void enterItem_flow_redefinition(SysMLv2Parser::Item_flow_redefinitionContext * /*ctx*/) override { }
	void exitItem_flow_redefinition(SysMLv2Parser::Item_flow_redefinitionContext * /*ctx*/) override { }

	void enterValue_part(SysMLv2Parser::Value_partContext * /*ctx*/) override { }
	void exitValue_part(SysMLv2Parser::Value_partContext * /*ctx*/) override { }

	void enterFeature_value(SysMLv2Parser::Feature_valueContext * /*ctx*/) override { }
	void exitFeature_value(SysMLv2Parser::Feature_valueContext * /*ctx*/) override { }

	void enterFeature_assignment(SysMLv2Parser::Feature_assignmentContext * /*ctx*/) override { }
	void exitFeature_assignment(SysMLv2Parser::Feature_assignmentContext * /*ctx*/) override { }

	void enterMultiplicity(SysMLv2Parser::MultiplicityContext * /*ctx*/) override { }
	void exitMultiplicity(SysMLv2Parser::MultiplicityContext * /*ctx*/) override { }

	void enterMultiplicity_subset(SysMLv2Parser::Multiplicity_subsetContext * /*ctx*/) override { }
	void exitMultiplicity_subset(SysMLv2Parser::Multiplicity_subsetContext * /*ctx*/) override { }

	void enterOwned_multiplicity_range(SysMLv2Parser::Owned_multiplicity_rangeContext * /*ctx*/) override { }
	void exitOwned_multiplicity_range(SysMLv2Parser::Owned_multiplicity_rangeContext * /*ctx*/) override { }

	void enterMultiplicity_bounds(SysMLv2Parser::Multiplicity_boundsContext * /*ctx*/) override { }
	void exitMultiplicity_bounds(SysMLv2Parser::Multiplicity_boundsContext * /*ctx*/) override { }

	void enterMultiplicity_expression_member(SysMLv2Parser::Multiplicity_expression_memberContext * /*ctx*/) override { }
	void exitMultiplicity_expression_member(SysMLv2Parser::Multiplicity_expression_memberContext * /*ctx*/) override { }

	void enterInternal_multiplicity_expression_member(SysMLv2Parser::Internal_multiplicity_expression_memberContext * /*ctx*/) override { }
	void exitInternal_multiplicity_expression_member(SysMLv2Parser::Internal_multiplicity_expression_memberContext * /*ctx*/) override { }

	void enterMetaclass(SysMLv2Parser::MetaclassContext * /*ctx*/) override { }
	void exitMetaclass(SysMLv2Parser::MetaclassContext * /*ctx*/) override { }

	void enterPrefix_metadata_feature(SysMLv2Parser::Prefix_metadata_featureContext * /*ctx*/) override { }
	void exitPrefix_metadata_feature(SysMLv2Parser::Prefix_metadata_featureContext * /*ctx*/) override { }

	void enterMetadata_feature(SysMLv2Parser::Metadata_featureContext * /*ctx*/) override { }
	void exitMetadata_feature(SysMLv2Parser::Metadata_featureContext * /*ctx*/) override { }

	void enterMetadata_feature_declaration(SysMLv2Parser::Metadata_feature_declarationContext * /*ctx*/) override { }
	void exitMetadata_feature_declaration(SysMLv2Parser::Metadata_feature_declarationContext * /*ctx*/) override { }

	void enterMetadata_body_element(SysMLv2Parser::Metadata_body_elementContext * /*ctx*/) override { }
	void exitMetadata_body_element(SysMLv2Parser::Metadata_body_elementContext * /*ctx*/) override { }

	void enterMetadata_body_feature_member(SysMLv2Parser::Metadata_body_feature_memberContext * /*ctx*/) override { }
	void exitMetadata_body_feature_member(SysMLv2Parser::Metadata_body_feature_memberContext * /*ctx*/) override { }

	void enterMetadata_body_feature(SysMLv2Parser::Metadata_body_featureContext * /*ctx*/) override { }
	void exitMetadata_body_feature(SysMLv2Parser::Metadata_body_featureContext * /*ctx*/) override { }

	void enterLibrary_package(SysMLv2Parser::Library_packageContext * /*ctx*/) override { }
	void exitLibrary_package(SysMLv2Parser::Library_packageContext * /*ctx*/) override { }

	void enterPackage_declaration(SysMLv2Parser::Package_declarationContext * /*ctx*/) override { }
	void exitPackage_declaration(SysMLv2Parser::Package_declarationContext * ctx) override;

	void enterElement_filter_member(SysMLv2Parser::Element_filter_memberContext * /*ctx*/) override { }
	void exitElement_filter_member(SysMLv2Parser::Element_filter_memberContext * /*ctx*/) override { }

	void enterMeta_assignment(SysMLv2Parser::Meta_assignmentContext * /*ctx*/) override { }
	void exitMeta_assignment(SysMLv2Parser::Meta_assignmentContext * /*ctx*/) override { }

	void enterTyped_by(SysMLv2Parser::Typed_byContext * /*ctx*/) override { }
	void exitTyped_by(SysMLv2Parser::Typed_byContext * /*ctx*/) override { }


	void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
	void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
	void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
	void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }


	std::vector<DigitalTwin::Model::IDigitalTwinElement*> getElements() const;
private:
	DigitalTwin::Model::SupportedTypes getTypeForString(std::string type);

	DigitalTwin::Model::IDigitalTwinElement* getElementWithName(std::string name);

	std::vector<DigitalTwin::Model::IDigitalTwinElement*> Elements;
	std::stack< DigitalTwin::Model::IDigitalTwinElement*> ParentStack;
};

