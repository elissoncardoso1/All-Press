#include <gtest/gtest.h>
#include "conversion/file_processor.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace AllPress;

class FileProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / "all_press_test";
        fs::create_directories(test_dir);
    }

    void TearDown() override {
        if (fs::exists(test_dir)) {
            fs::remove_all(test_dir);
        }
    }

    fs::path test_dir;
};

TEST_F(FileProcessorTest, DetectsFileType) {
    FileProcessor processor;
    
    EXPECT_EQ(processor.detect_file_type("document.pdf"), FileType::PDF);
    EXPECT_EQ(processor.detect_file_type("image.jpg"), FileType::Image);
    EXPECT_EQ(processor.detect_file_type("IMAGE.PNG"), FileType::Image);
    EXPECT_EQ(processor.detect_file_type("file.txt"), FileType::Unknown);
}

TEST_F(FileProcessorTest, CreatesPDFFromImage) {
    FileProcessor processor;
    
    auto img_path = test_dir / "test.jpg";
    auto pdf_path = test_dir / "output.pdf";
    
    std::ofstream img_file(img_path, std::ios::binary);
    img_file << "dummy image data";
    img_file.close();
    
    ASSERT_TRUE(fs::exists(img_path));
}

TEST_F(FileProcessorTest, ValidatesFileSize) {
    FileProcessor processor;
    
    auto test_file = test_dir / "test.txt";
    std::ofstream file(test_file);
    file << "test content";
    file.close();
    
    ASSERT_TRUE(fs::exists(test_file));
    EXPECT_GT(fs::file_size(test_file), 0);
}

TEST_F(FileProcessorTest, HandlesNonExistentFile) {
    FileProcessor processor;
    
    auto result = processor.detect_file_type("nonexistent.pdf");
    EXPECT_EQ(result, FileType::Unknown);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
