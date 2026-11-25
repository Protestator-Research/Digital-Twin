// Tests of the settings of the client, especially the choice of the MQTT login.
// QStandardPaths test mode is enabled, so the real configuration of the user is never read or written.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>

#include "../src/Models/DigitalTwinClientSettings.h"

using DigitalTwin::Client::DigitalTwinClientSettings;

class ClientSettingsTest : public ::testing::Test {
protected:
    static QString configFile() {
        return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.ini";
    }

    void SetUp() override {
        // The location of the real configuration, to make sure that it is not the one used by the tests.
        QStandardPaths::setTestModeEnabled(false);
        const QString realConfigFile = configFile();
        QStandardPaths::setTestModeEnabled(true);
        QCoreApplication::setApplicationName("DigitalTwinClientSettingsTest");
        // Every test starts without a configuration file in the test mode location.
        ASSERT_TRUE(QStandardPaths::isTestModeEnabled());
        ASSERT_NE(configFile(), realConfigFile) << "not in the test mode location";
        QFile::remove(configFile());
    }

    void TearDown() override {
        QFile::remove(configFile());
    }
};

TEST_F(ClientSettingsTest, DefaultUsesTheRestLogin) {
    DigitalTwinClientSettings settings;
    EXPECT_TRUE(settings.getMQTTUseRESTLogin());
    EXPECT_EQ(settings.getMQTTLoginUserAsString(), settings.getRESTUserAsString());
    EXPECT_EQ(settings.getMQTTLoginPasswordAsString(), settings.getRESTPasswordAsString());
}

TEST_F(ClientSettingsTest, OwnLoginIsUsedIfRestLoginIsNotUsed) {
    DigitalTwinClientSettings settings;
    const std::string restUser = settings.getRESTUserAsString();
    const std::string restPassword = settings.getRESTPasswordAsString();

    settings.setMQTTUser("mqtt-user");
    settings.setMQTTPassword("mqtt-secret");
    settings.setMQTTUseRESTLogin(false);

    EXPECT_FALSE(settings.getMQTTUseRESTLogin());
    EXPECT_EQ(settings.getMQTTLoginUserAsString(), "mqtt-user");
    EXPECT_EQ(settings.getMQTTLoginPasswordAsString(), "mqtt-secret");
    // The REST login is not touched.
    EXPECT_EQ(settings.getRESTUserAsString(), restUser);
    EXPECT_EQ(settings.getRESTPasswordAsString(), restPassword);

    // The stored own values stay available, if the REST login is used again.
    settings.setMQTTUseRESTLogin(true);
    EXPECT_EQ(settings.getMQTTLoginUserAsString(), restUser);
    EXPECT_EQ(settings.getMQTTLoginPasswordAsString(), restPassword);
    EXPECT_EQ(settings.getMQTTUser(), "mqtt-user");
    EXPECT_EQ(settings.getMQTTPassword(), "mqtt-secret");
}

TEST_F(ClientSettingsTest, OwnLoginSurvivesSavingAndLoading) {
    {
        DigitalTwinClientSettings settings;
        settings.setMQTTUser("mqtt-user");
        settings.setMQTTPassword("mqtt-secret");
        settings.setMQTTUseRESTLogin(false);
    }

    // A new object reads the values from the file.
    DigitalTwinClientSettings settings;
    EXPECT_FALSE(settings.getMQTTUseRESTLogin());
    EXPECT_EQ(settings.getMQTTLoginUserAsString(), "mqtt-user");
    EXPECT_EQ(settings.getMQTTLoginPasswordAsString(), "mqtt-secret");
}

TEST_F(ClientSettingsTest, OldConfigurationWithoutTheKeyUsesTheRestLogin) {
    {
        // A configuration of an older version: the connection settings exist, the MQTT login keys do not.
        QSettings old(configFile(), QSettings::IniFormat);
        old.beginGroup("Connection Settings");
        old.setValue("RESTServer", "example.org");
        old.setValue("RESTPort", "9090");
        old.setValue("RESTLogin", "old@example.org");
        old.setValue("RESTPassword", "old-secret");
        old.setValue("RESTFolder", "/agila-server/");
        old.setValue("MQTTProtocolVersion", "3.1.1");
        old.endGroup();
        old.sync();
        ASSERT_FALSE(old.contains("Connection Settings/MQTTUseRESTLogin"));
    }

    DigitalTwinClientSettings settings;
    EXPECT_TRUE(settings.getMQTTUseRESTLogin());
    EXPECT_EQ(settings.getMQTTLoginUserAsString(), "old@example.org");
    EXPECT_EQ(settings.getMQTTLoginPasswordAsString(), "old-secret");
}

TEST_F(ClientSettingsTest, ResetToDefaultRestoresTheMqttLogin) {
    DigitalTwinClientSettings settings;
    settings.setMQTTUser("mqtt-user");
    settings.setMQTTPassword("mqtt-secret");
    settings.setMQTTUseRESTLogin(false);

    settings.resetToDefault();

    EXPECT_TRUE(settings.getMQTTUseRESTLogin());
    EXPECT_TRUE(settings.getMQTTUser().isEmpty());
    EXPECT_TRUE(settings.getMQTTPassword().isEmpty());
    EXPECT_EQ(settings.getMQTTLoginUserAsString(), settings.getRESTUserAsString());
}
