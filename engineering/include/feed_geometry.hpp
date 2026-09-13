#pragma once
#include "head_geometry.hpp"
namespace starz::engineering {
TopoDS_Shape feedShaftShape();
TopoDS_Shape feedHousingShape();
TopoDS_Shape feedRollerShape();
enum class FeedRole { motor_driven,belt_follower,passive };
std::vector<HeadPart> feedCartridge(FeedRole role=FeedRole::motor_driven);
void checkFeedCartridge(const std::vector<HeadPart>& parts);
void checkFeedAssemblyPaths(const std::vector<HeadPart>& parts);
void checkFeedMachiningFeatures();
void rejectUnfinishedFeedMachining();
void rejectOldFeedAssemblyPaths(bool closed_washer_slot);
std::vector<HeadPart> feedCassette(double wire_diameter_mm);
}