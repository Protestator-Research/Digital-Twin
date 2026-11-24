// Tests of the state of the model (value encoding, change notification, tree walk, change sets, snapshot and
// restore). They need no backend: a DigitalTwinModel cannot be built offline, so the parts that it is made of
// (walkModelTree, StateTracking) are tested on hand-built Package and Component objects.
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include "Model/Entities/Component.h"
#include "Model/Entities/Package.h"
#include "Model/Entities/Port.h"
#include "Model/Entities/Variables/BooleanVariable.h"
#include "Model/Entities/Variables/ComplexVariable.h"
#include "Model/Entities/Variables/IntegerVariable.h"
#include "Model/Entities/Variables/PositiveVariable.h"
#include "Model/Entities/Variables/RationalVariable.h"
#include "Model/Entities/Variables/RealVariable.h"
#include "Model/Entities/Variables/StringVariable.h"
#include "Model/State/ModelTree.h"
#include "Model/State/StateTracking.h"
#include "Model/State/StateTypes.h"
#include "Model/State/ValueCodec.h"

using namespace DigitalTwin::Model;

namespace {
	// ---------------------------------------------------------------------------------------------------------
	// Value encoding
	// ---------------------------------------------------------------------------------------------------------

	TEST(ValueCodec, RealIsStoredInNum) {
		const auto value = ValueCodec::encodeReal(1.5);
		EXPECT_EQ(value.type, ValueType::Real);
		ASSERT_TRUE(value.num.has_value());
		EXPECT_DOUBLE_EQ(*value.num, 1.5);
		EXPECT_FALSE(value.text.has_value());
		EXPECT_FALSE(value.json.has_value());
		double decoded = 0;
		ASSERT_TRUE(ValueCodec::decodeReal(value, decoded));
		EXPECT_DOUBLE_EQ(decoded, 1.5);
	}

	TEST(ValueCodec, NonFiniteRealIsWrittenAsZero) {
		EXPECT_EQ(*ValueCodec::encodeReal(std::numeric_limits<double>::quiet_NaN()).num, 0.0);
		EXPECT_EQ(*ValueCodec::encodeReal(std::numeric_limits<double>::infinity()).num, 0.0);
		EXPECT_EQ(*ValueCodec::encodeReal(-std::numeric_limits<double>::infinity()).num, 0.0);
	}

	TEST(ValueCodec, SmallIntegerIsStoredInNum) {
		for (const int64_t number : {int64_t{0}, int64_t{-7}, int64_t{42}, ValueCodec::MAX_EXACT_INTEGER, -ValueCodec::MAX_EXACT_INTEGER}) {
			const auto value = ValueCodec::encodeInteger(number);
			EXPECT_EQ(value.type, ValueType::Integer);
			ASSERT_TRUE(value.num.has_value()) << number;
			EXPECT_FALSE(value.text.has_value()) << number;
			int64_t decoded = 1;
			ASSERT_TRUE(ValueCodec::decodeInteger(value, decoded));
			EXPECT_EQ(decoded, number);
		}
	}

	TEST(ValueCodec, LargeIntegerIsStoredAsExactDigits) {
		const int64_t beyond = ValueCodec::MAX_EXACT_INTEGER + 1;   // 2^53 + 1 is not a double
		const auto value = ValueCodec::encodeInteger(beyond);
		EXPECT_FALSE(value.num.has_value());
		ASSERT_TRUE(value.text.has_value());
		EXPECT_EQ(*value.text, "9007199254740993");
		int64_t decoded = 0;
		ASSERT_TRUE(ValueCodec::decodeInteger(value, decoded));
		EXPECT_EQ(decoded, beyond);

		const auto smallest = ValueCodec::encodeInteger(std::numeric_limits<int64_t>::min());
		ASSERT_TRUE(smallest.text.has_value());
		ASSERT_TRUE(ValueCodec::decodeInteger(smallest, decoded));
		EXPECT_EQ(decoded, std::numeric_limits<int64_t>::min());
	}

	TEST(ValueCodec, IntegerDecodeRejectsFractionsAndGarbage) {
		VariableValue value;
		value.type = ValueType::Integer;
		value.num = 1.5;
		int64_t decoded = 99;
		EXPECT_FALSE(ValueCodec::decodeInteger(value, decoded));
		value.num.reset();
		value.text = "12x";
		EXPECT_FALSE(ValueCodec::decodeInteger(value, decoded));
		value.text = "";
		EXPECT_FALSE(ValueCodec::decodeInteger(value, decoded));
		EXPECT_EQ(decoded, 99);
	}

	TEST(ValueCodec, BooleanIsZeroOrOne) {
		EXPECT_EQ(*ValueCodec::encodeBoolean(true).num, 1.0);
		EXPECT_EQ(*ValueCodec::encodeBoolean(false).num, 0.0);
		bool decoded = false;
		ASSERT_TRUE(ValueCodec::decodeBoolean(ValueCodec::encodeBoolean(true), decoded));
		EXPECT_TRUE(decoded);
	}

	TEST(ValueCodec, RationalIsStoredAsFractionText) {
		const auto value = ValueCodec::encodeRational(-3, 4);
		EXPECT_EQ(value.type, ValueType::Rational);
		ASSERT_TRUE(value.text.has_value());
		EXPECT_EQ(*value.text, "-3/4");
		int64_t numerator = 0;
		int64_t denominator = 0;
		ASSERT_TRUE(ValueCodec::decodeRational(value, numerator, denominator));
		EXPECT_EQ(numerator, -3);
		EXPECT_EQ(denominator, 4);

		VariableValue broken;
		broken.type = ValueType::Rational;
		for (const char* text : {"3", "3/0", "a/b", "/4", "3/"}) {
			broken.text = text;
			EXPECT_FALSE(ValueCodec::decodeRational(broken, numerator, denominator)) << text;
		}
	}

	TEST(ValueCodec, StringIsStoredInText) {
		const auto value = ValueCodec::encodeString("h\xC3\xA4llo \"quoted\"");
		EXPECT_EQ(value.type, ValueType::String);
		std::string decoded;
		ASSERT_TRUE(ValueCodec::decodeString(value, decoded));
		EXPECT_EQ(decoded, "h\xC3\xA4llo \"quoted\"");
		ASSERT_TRUE(ValueCodec::decodeString(ValueCodec::encodeString(""), decoded));
		EXPECT_EQ(decoded, "");
	}

	TEST(ValueCodec, ComplexIsStoredAsJson) {
		const auto value = ValueCodec::encodeComplex(1.0, -2.5);
		EXPECT_EQ(value.type, ValueType::Complex);
		ASSERT_TRUE(value.json.has_value());
		EXPECT_EQ(*value.json, R"({"imaginaryValue":-2.5,"realValue":1.0})");
		double real = 0;
		double imaginary = 0;
		ASSERT_TRUE(ValueCodec::decodeComplex(value, real, imaginary));
		EXPECT_DOUBLE_EQ(real, 1.0);
		EXPECT_DOUBLE_EQ(imaginary, -2.5);

		const auto nonFinite = ValueCodec::encodeComplex(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity());
		ASSERT_TRUE(ValueCodec::decodeComplex(nonFinite, real, imaginary));
		EXPECT_EQ(real, 0.0);
		EXPECT_EQ(imaginary, 0.0);
	}

	TEST(ValueCodec, DecodeRejectsAnotherType) {
		double real = 0;
		int64_t integer = 0;
		bool boolean = false;
		std::string text;
		EXPECT_FALSE(ValueCodec::decodeReal(ValueCodec::encodeInteger(1), real));
		EXPECT_FALSE(ValueCodec::decodeInteger(ValueCodec::encodeReal(1.0), integer));
		EXPECT_FALSE(ValueCodec::decodeBoolean(ValueCodec::encodeReal(1.0), boolean));
		EXPECT_FALSE(ValueCodec::decodeString(ValueCodec::encodeRational(1, 2), text));
		EXPECT_FALSE(ValueCodec::decodeComplex(ValueCodec::encodeString("{}"), real, real));
	}

	// ---------------------------------------------------------------------------------------------------------
	// Variables: state value and change notification
	// ---------------------------------------------------------------------------------------------------------

	class RecordingObserver : public IVariableObserver {
	public:
		std::vector<std::string> Names;
		void variableChanged(IVariable& variable) override { Names.push_back(variable.getName()); }
	};

	TEST(VariableState, EveryClassRoundTripsItsStateValue) {
		RealVariable real("r", 2.5);
		IntegerVariable integer("i", 7);
		BooleanVariable boolean("b", true);
		RationalVariable rational("q", 3, 4);
		ComplexVariable complex("c", 1.0, 2.0);
		StringVariable string("s", "text");
		PositiveVariable positive("p", 9u);

		EXPECT_EQ(real.getValueType(), ValueType::Real);
		EXPECT_EQ(integer.getValueType(), ValueType::Integer);
		EXPECT_EQ(boolean.getValueType(), ValueType::Boolean);
		EXPECT_EQ(rational.getValueType(), ValueType::Rational);
		EXPECT_EQ(complex.getValueType(), ValueType::Complex);
		EXPECT_EQ(string.getValueType(), ValueType::String);
		EXPECT_EQ(positive.getValueType(), ValueType::Integer);

		RealVariable real2("r");
		IntegerVariable integer2("i");
		BooleanVariable boolean2("b");
		RationalVariable rational2("q");
		ComplexVariable complex2("c");
		StringVariable string2("s");
		PositiveVariable positive2("p");
		EXPECT_TRUE(real2.setStateValue(real.getStateValue()));
		EXPECT_TRUE(integer2.setStateValue(integer.getStateValue()));
		EXPECT_TRUE(boolean2.setStateValue(boolean.getStateValue()));
		EXPECT_TRUE(rational2.setStateValue(rational.getStateValue()));
		EXPECT_TRUE(complex2.setStateValue(complex.getStateValue()));
		EXPECT_TRUE(string2.setStateValue(string.getStateValue()));
		EXPECT_TRUE(positive2.setStateValue(positive.getStateValue()));

		EXPECT_DOUBLE_EQ(real2.getValue(), 2.5);
		EXPECT_EQ(integer2.getValue(), 7);
		EXPECT_TRUE(boolean2.getValue());
		EXPECT_EQ(rational2.getValue(), boost::rational<long>(3, 4));
		EXPECT_EQ(complex2.getValue(), std::complex<double>(1.0, 2.0));
		EXPECT_EQ(string2.getValue(), "text");
		EXPECT_EQ(positive2.getValue(), 9u);
	}

	TEST(VariableState, WrongTypeOrRangeIsRejectedAndLeavesTheValue) {
		RealVariable real("r", 1.0);
		EXPECT_FALSE(real.setStateValue(ValueCodec::encodeString("x")));
		EXPECT_DOUBLE_EQ(real.getValue(), 1.0);

		PositiveVariable positive("p", 5u);
		EXPECT_FALSE(positive.setStateValue(ValueCodec::encodeInteger(-1)));
		EXPECT_EQ(positive.getValue(), 5u);

		IntegerVariable integer("i", 3);
		EXPECT_FALSE(integer.setStateValue(ValueCodec::encodeBoolean(true)));
		EXPECT_EQ(integer.getValue(), 3);

		RationalVariable rational("q", 1, 2);
		VariableValue zeroDenominator;
		zeroDenominator.type = ValueType::Rational;
		zeroDenominator.text = "1/0";
		EXPECT_FALSE(rational.setStateValue(zeroDenominator));
		EXPECT_EQ(rational.getValue(), boost::rational<long>(1, 2));
	}

	TEST(VariableState, DefaultPositiveIsReadable) {
		PositiveVariable positive("p");
		EXPECT_EQ(positive.getValue(), 0u);
	}

	TEST(VariableObserver, SetValueNotifiesTheVariableThenTheLinkedOnes) {
		RealVariable a("a");
		RealVariable b("b");
		RealVariable c("c");
		a.addLinkToVariable(&b);
		a.addLinkToVariable(&c);
		RecordingObserver observer;
		a.setObserver(&observer);
		b.setObserver(&observer);
		c.setObserver(&observer);

		a.setValue(3.0);
		// The linked variables are changed through setValueWithoutPropagation, which reports as well.
		EXPECT_EQ(observer.Names, (std::vector<std::string>{"a", "b", "c"}));
		EXPECT_DOUBLE_EQ(b.getValue(), 3.0);
	}

	TEST(VariableObserver, EveryClassNotifies) {
		RecordingObserver observer;
		RealVariable real("r");
		IntegerVariable integer("i");
		BooleanVariable boolean("b");
		RationalVariable rational("q");
		ComplexVariable complex("c");
		StringVariable string("s");
		PositiveVariable positive("p");
		for (IVariable* variable : std::vector<IVariable*>{&real, &integer, &boolean, &rational, &complex, &string, &positive})
			variable->setObserver(&observer);

		real.setValue(1.0);
		integer.setValue(1);
		boolean.setValue(true);
		rational.setValue(boost::rational<long>(1, 2));
		rational.setValue(1L, 3L);
		complex.setValue(std::complex<double>(1.0, 1.0));
		complex.setValue(2.0, 2.0);
		string.setValue("x");
		positive.setValue(1u);
		EXPECT_EQ(observer.Names.size(), 9u);

		// The state setter reports as well (it sets without propagation).
		observer.Names.clear();
		real.setStateValue(ValueCodec::encodeReal(5.0));
		EXPECT_EQ(observer.Names, std::vector<std::string>{"r"});
	}

	TEST(VariableObserver, RemovedObserverIsNotCalledAndCopyHasNone) {
		RecordingObserver observer;
		RealVariable a("a", 1.0);
		a.setObserver(&observer);
		std::unique_ptr<IVariable> copy(a.copy());
		EXPECT_EQ(copy->getObserver(), nullptr);
		a.setObserver(nullptr);
		a.setValue(2.0);
		EXPECT_TRUE(observer.Names.empty());
	}

	// ---------------------------------------------------------------------------------------------------------
	// Tree walk
	// ---------------------------------------------------------------------------------------------------------

	/**
	 * A model like the builder makes it, without a backend:
	 *   Pkg: measurable pm, controllable pc, attribute pa; component definition Def (measurable temp, controllable set,
	 *        attribute label); instances i1 and i2 of Def.
	 *   Root: measurable x, nested component Sub (measurable y, attribute note), port P (attribute pp).
	 * Nothing is deleted: Component's destructor deletes the measurables that Component::instantiate shares, which
	 * would be a double free.
	 */
	struct Fixture {
		Package Pkg{"Pkg"};
		Component* Root = new Component("Root");
		Component* Sub = new Component("Sub");
		Port* P = new Port("P");
		RealVariable* PM = new RealVariable("pm");
		IntegerVariable* PC = new IntegerVariable("pc");
		StringVariable* PA = new StringVariable("pa");
		RealVariable* X = new RealVariable("x");
		RealVariable* Y = new RealVariable("y");
		StringVariable* Note = new StringVariable("note");
		StringVariable* PP = new StringVariable("pp");

		std::map<std::string, Package*> Packages;
		std::map<std::string, Component*> Components;
		std::map<std::string, Port*> Ports;

		Fixture()
		{
			Pkg.appendMeasurable(PM);
			Pkg.appendControllable(PC);
			Pkg.appendAttribute(PA);
			auto* definition = new Component("Def");
			definition->appendMeasurable(new RealVariable("temp"));
			definition->appendControllable(new IntegerVariable("set"));
			definition->appendAttribute(new StringVariable("label"));
			Pkg.appendComponent(definition);
			Pkg.instantiateComponent("i1", "Def");
			Pkg.instantiateComponent("i2", "Def");

			Root->appendMeasurable(X);
			Sub->appendMeasurable(Y);
			Sub->appendAttribute(Note);
			Root->appendComponent(Sub);
			P->appendAttribute(PP);
			Root->appendPort(P);

			Packages["Pkg"] = &Pkg;
			Components["Root"] = Root;
		}

		std::vector<ModelTreeEntry> walk() const { return walkModelTree(Packages, Components, Ports); }
	};

	const ModelTreeEntry* find(const std::vector<ModelTreeEntry>& entries, const std::string& path)
	{
		for (const auto& entry : entries)
			if (entry.info.path == path)
				return &entry;
		return nullptr;
	}

	TEST(ModelTree, ListsEveryElementWithKindAndType) {
		Fixture f;
		const auto entries = f.walk();
		const auto expect = [&](const std::string& path, ElementKind kind, std::optional<ValueType> type) {
			const auto* entry = find(entries, path);
			ASSERT_NE(entry, nullptr) << path;
			EXPECT_EQ(entry->info.kind, kind) << path;
			EXPECT_EQ(entry->info.valueType, type) << path;
			EXPECT_EQ(entry->variable != nullptr, type.has_value()) << path;
		};
		expect("Pkg", ElementKind::Package, std::nullopt);
		expect("Pkg/pm", ElementKind::Measurable, ValueType::Real);
		expect("Pkg/pc", ElementKind::Controllable, ValueType::Integer);
		expect("Pkg/pa", ElementKind::Attribute, ValueType::String);
		expect("Pkg/i1", ElementKind::Component, std::nullopt);
		expect("Pkg/i1/temp", ElementKind::Measurable, ValueType::Real);
		expect("Pkg/i1/set", ElementKind::Controllable, ValueType::Integer);
		expect("Pkg/i1/label", ElementKind::Attribute, ValueType::String);
		expect("Pkg/i2/set", ElementKind::Controllable, ValueType::Integer);
		expect("Root", ElementKind::Component, std::nullopt);
		expect("Root/x", ElementKind::Measurable, ValueType::Real);
		expect("Root/Sub", ElementKind::Component, std::nullopt);
		expect("Root/Sub/y", ElementKind::Measurable, ValueType::Real);
		expect("Root/Sub/note", ElementKind::Attribute, ValueType::String);
		expect("Root/P", ElementKind::Port, std::nullopt);
		expect("Root/P/pp", ElementKind::Attribute, ValueType::String);
		// The definition is a template without state, only its instances are part of the tree.
		EXPECT_EQ(find(entries, "Pkg/Def"), nullptr);
		EXPECT_EQ(find(entries, "Pkg/Def/temp"), nullptr);
	}

	TEST(ModelTree, ParentsComeFirstAndPathsAreUnique) {
		Fixture f;
		const auto entries = f.walk();
		std::set<std::string> seen;
		for (const auto& entry : entries) {
			const auto& info = entry.info;
			EXPECT_TRUE(seen.insert(info.path).second) << "duplicate " << info.path;
			if (info.parentPath.empty()) {
				EXPECT_EQ(info.path, info.name);
			}
			else {
				EXPECT_TRUE(seen.count(info.parentPath)) << info.path << " precedes its parent " << info.parentPath;
				EXPECT_EQ(info.path, info.parentPath + "/" + info.name);
			}
		}
	}

	TEST(ModelTree, VariablePathsEqualTheMqttTopicPaths) {
		Fixture f;
		const auto entries = f.walk();
		const auto check = [&](const std::string& root, const std::vector<std::pair<std::string, IVariable*>>& mqtt) {
			ASSERT_FALSE(mqtt.empty());
			for (const auto& [topic, variable] : mqtt) {
				const auto* entry = find(entries, root + "/" + topic);
				ASSERT_NE(entry, nullptr) << root << "/" << topic;
				EXPECT_EQ(entry->variable, variable) << root << "/" << topic;
			}
		};
		check("Pkg", f.Pkg.getAllMQTTVariables());
		check("Root", f.Root->getAllMQTTVariables());
	}

	TEST(ModelTree, SharedMeasurableIsListedUnderEveryPath) {
		Fixture f;
		const auto entries = f.walk();
		const auto* first = find(entries, "Pkg/i1/temp");
		const auto* second = find(entries, "Pkg/i2/temp");
		ASSERT_NE(first, nullptr);
		ASSERT_NE(second, nullptr);
		EXPECT_EQ(first->variable, second->variable);
		// Controllables and attributes of the instances are own copies / the shared ones, but never the definition's.
		EXPECT_NE(find(entries, "Pkg/i1/set")->variable, find(entries, "Pkg/i2/set")->variable);
	}

	TEST(ModelTree, FirstElementWinsOnAPathCollision) {
		Component root("C");
		auto* controllable = new IntegerVariable("v");
		auto* measurable = new RealVariable("v");
		root.appendControllable(controllable);
		root.appendMeasurable(measurable);
		std::map<std::string, Component*> components{{"C", &root}};
		const auto entries = walkModelTree({}, components, {});
		const auto* entry = find(entries, "C/v");
		ASSERT_NE(entry, nullptr);
		EXPECT_EQ(entry->variable, controllable);   // the precedence of resolveVariable
		EXPECT_EQ(std::count_if(entries.begin(), entries.end(), [](const auto& e) { return e.info.path == "C/v"; }), 1);
	}

	TEST(ModelTree, EmptyModelHasNoElements) {
		EXPECT_TRUE(walkModelTree({}, {}, {}).empty());
	}

	// ---------------------------------------------------------------------------------------------------------
	// Change sets, snapshot and restore
	// ---------------------------------------------------------------------------------------------------------

	TEST(StateTracking, ChangeSetHasTheAddressedVariableFirstThenTheDerivedOnes) {
		Fixture f;
		f.X->addLinkToVariable(f.Y);
		f.Y->addLinkToVariable(f.X);
		StateTracking tracking(f.walk());
		{
			StateTracking::Scope scope(tracking);
			f.Y->setValue(4.5);
			const auto changes = tracking.collectChanges(f.Y, "Root/Sub/y");
			ASSERT_EQ(changes.size(), 2u);
			EXPECT_EQ(changes[0].path, "Root/Sub/y");
			EXPECT_FALSE(changes[0].derived);
			EXPECT_EQ(changes[1].path, "Root/x");
			EXPECT_TRUE(changes[1].derived);
			EXPECT_DOUBLE_EQ(*changes[0].value.num, 4.5);
			EXPECT_DOUBLE_EQ(*changes[1].value.num, 4.5);
		}
	}

	TEST(StateTracking, SharedVariableIsReportedUnderEveryPath) {
		Fixture f;
		StateTracking tracking(f.walk());
		auto* shared = static_cast<RealVariable*>(find(tracking.entries(), "Pkg/i1/temp")->variable);
		StateTracking::Scope scope(tracking);
		shared->setValue(21.0);
		const auto changes = tracking.collectChanges(shared, "Pkg/i2/temp");
		ASSERT_EQ(changes.size(), 2u);
		EXPECT_EQ(changes[0].path, "Pkg/i2/temp");   // the path the client addressed
		EXPECT_FALSE(changes[0].derived);
		EXPECT_EQ(changes[1].path, "Pkg/i1/temp");
		EXPECT_TRUE(changes[1].derived);
	}

	TEST(StateTracking, UnknownAddressedPathFallsBackToTheFirstPath) {
		Fixture f;
		StateTracking tracking(f.walk());
		StateTracking::Scope scope(tracking);
		f.X->setValue(1.0);
		const auto changes = tracking.collectChanges(f.X, "Root.x");   // an alias that resolveVariable accepts
		ASSERT_EQ(changes.size(), 1u);
		EXPECT_EQ(changes[0].path, "Root/x");
		EXPECT_FALSE(changes[0].derived);
	}

	TEST(StateTracking, ChangesOutsideAScopeAreNotRecorded) {
		Fixture f;
		StateTracking tracking(f.walk());
		f.X->setValue(1.0);
		{
			StateTracking::Scope scope(tracking);
		}
		EXPECT_TRUE(tracking.collectChanges(f.X, "Root/x").empty());
		f.X->setValue(2.0);
		EXPECT_TRUE(tracking.collectChanges(f.X, "Root/x").empty());
	}

	TEST(StateTracking, ScopeStartsEmptyAndVariablesOutsideTheTreeAreLeftOut) {
		Fixture f;
		RealVariable outside("outside");
		f.X->addLinkToVariable(&outside);
		StateTracking tracking(f.walk());
		StateTracking::Scope first(tracking);
		f.X->setValue(1.0);
		EXPECT_EQ(tracking.collectChanges(f.X, "Root/x").size(), 1u);   // `outside` has no path
		EXPECT_DOUBLE_EQ(outside.getValue(), 1.0);
	}

	TEST(StateTracking, VariableThatWasNotSetIsNotReported) {
		Fixture f;
		StateTracking tracking(f.walk());
		StateTracking::Scope scope(tracking);
		EXPECT_TRUE(tracking.collectChanges(f.X, "Root/x").empty());
	}

	TEST(StateTracking, DestructorRemovesTheObservers) {
		Fixture f;
		{
			StateTracking tracking(f.walk());
			EXPECT_NE(f.X->getObserver(), nullptr);
		}
		EXPECT_EQ(f.X->getObserver(), nullptr);
	}

	TEST(StateTracking, SnapshotListsEveryVariablePath) {
		Fixture f;
		f.X->setValue(1.25);
		f.Note->setValue("n");
		StateTracking tracking(f.walk());
		const auto state = tracking.snapshot();
		size_t variables = 0;
		for (const auto& entry : tracking.entries())
			if (entry.variable)
				++variables;
		EXPECT_EQ(state.size(), variables);
		const auto x = std::find_if(state.begin(), state.end(), [](const auto& s) { return s.path == "Root/x"; });
		ASSERT_NE(x, state.end());
		EXPECT_DOUBLE_EQ(*x->value.num, 1.25);
		const auto note = std::find_if(state.begin(), state.end(), [](const auto& s) { return s.path == "Root/Sub/note"; });
		ASSERT_NE(note, state.end());
		EXPECT_EQ(*note->value.text, "n");
	}

	TEST(StateTracking, RestoreSetsValuesWithoutPropagationAndSkipsMismatches) {
		Fixture f;
		f.X->addLinkToVariable(f.Y);
		f.Y->addLinkToVariable(f.X);
		StateTracking tracking(f.walk());

		const std::vector<VariableState> stored{
			{"Root/x", ValueCodec::encodeReal(9.0)},
			{"Root/Sub/note", ValueCodec::encodeString("restored")},
			{"Pkg/pc", ValueCodec::encodeInteger(11)},
			{"Pkg/pm", ValueCodec::encodeString("wrong type")},   // type mismatch
			{"Root/Sub/gone", ValueCodec::encodeReal(1.0)},       // unknown path
			{"Root", ValueCodec::encodeReal(1.0)},                // an element without value
		};
		EXPECT_EQ(tracking.restore(stored), 3u);
		EXPECT_DOUBLE_EQ(f.X->getValue(), 9.0);
		EXPECT_DOUBLE_EQ(f.Y->getValue(), 0.0);   // the linked variable was not touched
		EXPECT_EQ(f.Note->getValue(), "restored");
		EXPECT_EQ(static_cast<IntegerVariable*>(f.PC)->getValue(), 11);
		EXPECT_DOUBLE_EQ(f.PM->getValue(), 0.0);
	}

	TEST(StateTracking, SnapshotThenRestoreIsTheIdentity) {
		Fixture f;
		f.X->setValue(3.5);
		f.PC->setValue(8);
		f.PA->setValue("a");
		StateTracking tracking(f.walk());
		const auto saved = tracking.snapshot();
		f.X->setValue(0.0);
		f.PC->setValue(0);
		f.PA->setValue("");
		EXPECT_EQ(tracking.restore(saved), saved.size());
		EXPECT_DOUBLE_EQ(f.X->getValue(), 3.5);
		EXPECT_EQ(f.PC->getValue(), 8);
		EXPECT_EQ(f.PA->getValue(), "a");
	}
}
