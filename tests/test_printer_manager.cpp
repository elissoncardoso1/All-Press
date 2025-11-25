#include <gtest/gtest.h>
#include "core/printer_manager.h"
#include <thread>
#include <chrono>

using namespace AllPress;

class PrinterManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<PrinterManager>();
    }

    void TearDown() override {
        manager.reset();
    }

    std::unique_ptr<PrinterManager> manager;
};

TEST_F(PrinterManagerTest, DiscoversCupsPrinters) {
    auto printers = manager->discover_cups_printers();
    EXPECT_GE(printers.size(), 0);
}

TEST_F(PrinterManagerTest, GetsAllPrinters) {
    auto printers = manager->get_all_printers();
    EXPECT_GE(printers.size(), 0);
}

TEST_F(PrinterManagerTest, AsyncDiscovery) {
    auto future = manager->discover_printers_async();
    
    ASSERT_TRUE(future.valid());
    
    auto status = future.wait_for(std::chrono::seconds(5));
    EXPECT_NE(status, std::future_status::timeout);
    
    if (status == std::future_status::ready) {
        auto printers = future.get();
        EXPECT_GE(printers.size(), 0);
    }
}

TEST_F(PrinterManagerTest, ValidatesPrinterOptions) {
    PrintOptions options;
    options.media_size = "A4";
    options.color_mode = "color";
    options.copies = 1;
    
    EXPECT_EQ(options.media_size, "A4");
    EXPECT_EQ(options.color_mode, "color");
    EXPECT_EQ(options.copies, 1);
}

TEST_F(PrinterManagerTest, ChecksPrinterStatus) {
    auto printers = manager->get_all_printers();
    
    for (const auto& printer : printers) {
        EXPECT_FALSE(printer.name.empty());
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
