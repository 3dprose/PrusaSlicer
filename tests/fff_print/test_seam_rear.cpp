#include <libslic3r/Point.hpp>
#include <catch2/catch.hpp>
#include <libslic3r/GCode/SeamRear.hpp>
#include "test_data.hpp"
#include <fstream>

using namespace Slic3r;
using namespace Slic3r::Seams;

constexpr bool debug_files{false};

namespace RearTest {
Perimeters::Perimeter get_perimeter() {
    const double slice_z{1.0};
    const std::size_t layer_index{};
    std::vector<Vec2d> positions{{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.5, 1.0}, {0.0, 1.0}};
    std::vector<double> angles(positions.size(), -M_PI / 2.0);
    angles[3] = 0.0;
    std::vector<Perimeters::PointType> point_types(positions.size(), Perimeters::PointType::common);
    std::vector<Perimeters::PointClassification>
        point_classifications{positions.size(), Perimeters::PointClassification::common};
    std::vector<Perimeters::AngleType> angle_type(positions.size(), Perimeters::AngleType::concave);
    angle_type[3] = Perimeters::AngleType::smooth;

    return {
        slice_z,
        layer_index,
        std::move(positions),
        std::move(angles),
        std::move(point_types),
        std::move(point_classifications),
        std::move(angle_type)};
}
} // namespace RearTest

TEST_CASE("StraightLine operator places seam point near the prefered position", "[Seams][SeamRear]") {
    const Rear::Impl::StraightLine rearest{Vec2d{0.7, 2.0}};
    std::optional<SeamChoice> choice{rearest(RearTest::get_perimeter(), Perimeters::PointType::common, Perimeters::PointClassification::common)};

    REQUIRE(choice);
    CHECK(scaled(choice->position) == scaled(Vec2d{0.7, 1.0}));
    CHECK(choice->previous_index == 2);
    CHECK(choice->next_index == 3);
}

TEST_CASE_METHOD(Test::SeamsFixture, "Generate rear seam", "[Seams][SeamRear][Integration]") {
    Shells::Shells<> perimeters{
        Perimeters::create_perimeters(shell_polygons, layer_infos, painting, params.perimeter)};
    Shells::Shells<> shell_perimeters;
    shell_perimeters.push_back(std::move(perimeters[shell_index]));
    const std::vector<std::vector<SeamPerimeterChoice>> seam{
        Rear::get_object_seams(std::move(shell_perimeters), params.rear_project_threshold)};
    REQUIRE(seam.size() == 125);

    if constexpr (debug_files) {
        std::ofstream csv{"rear_seam.csv"};
        Test::serialize_seam(csv, seam);
    }
}
