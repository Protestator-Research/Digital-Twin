//
// Created by Moritz Herzog on 07.08.24.
//
#include <gtest/gtest.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <entities/DigitalTwinEntity.h>
#include <sstream>
#include <iostream>

TEST(TestSerilization, TestDigitalTwinEntity){
    auto dtId = boost::uuids::random_generator()();
    auto proId = boost::uuids::random_generator()();
    PHYSICAL_TWIN_COMMUNICATION::DigitalTwinEntity entity(dtId,proId);

    std::ostringstream stream;
    stream << "{\"projectID\":\""<<boost::uuids::to_string(entity.projectId())
            <<"\",\"twinID\":\""<<boost::uuids::to_string(entity.digitalTwinId())<<"\"}";


    EXPECT_EQ(entity.serialize(),stream.str());

}