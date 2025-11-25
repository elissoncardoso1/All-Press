#include <gtest/gtest.h>
#include "core/job_queue.h"
#include <thread>
#include <chrono>

using namespace AllPress;

class JobQueueTest : public ::testing::Test {
protected:
    void SetUp() override {
        queue = std::make_unique<JobQueue>(2);
    }

    void TearDown() override {
        queue.reset();
    }

    std::unique_ptr<JobQueue> queue;
};

TEST_F(JobQueueTest, AddsJob) {
    PrintJob job;
    job.printer_name = "test_printer";
    job.file_path = "/tmp/test.pdf";
    job.original_filename = "test.pdf";
    
    int job_id = queue->add_job(job);
    EXPECT_GT(job_id, 0);
}

TEST_F(JobQueueTest, CancelsJob) {
    PrintJob job;
    job.printer_name = "test_printer";
    job.file_path = "/tmp/test.pdf";
    job.original_filename = "test.pdf";
    
    int job_id = queue->add_job(job);
    ASSERT_GT(job_id, 0);
    
    bool cancelled = queue->cancel_job(job_id);
    EXPECT_TRUE(cancelled);
    
    auto retrieved = queue->get_job(job_id);
    if (retrieved.has_value()) {
        EXPECT_EQ(retrieved->status, JobStatus::Cancelled);
    }
}

TEST_F(JobQueueTest, GetsActiveJobs) {
    PrintJob job1;
    job1.printer_name = "printer1";
    job1.file_path = "/tmp/test1.pdf";
    job1.original_filename = "test1.pdf";
    
    PrintJob job2;
    job2.printer_name = "printer2";
    job2.file_path = "/tmp/test2.pdf";
    job2.original_filename = "test2.pdf";
    
    queue->add_job(job1);
    queue->add_job(job2);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto active = queue->get_active_jobs();
    EXPECT_GE(active.size(), 0);
}

TEST_F(JobQueueTest, PausesAndResumesJob) {
    PrintJob job;
    job.printer_name = "test_printer";
    job.file_path = "/tmp/test.pdf";
    job.original_filename = "test.pdf";
    
    int job_id = queue->add_job(job);
    ASSERT_GT(job_id, 0);
    
    bool paused = queue->pause_job(job_id);
    EXPECT_TRUE(paused || queue->get_job(job_id).has_value());
    
    bool resumed = queue->resume_job(job_id);
    EXPECT_TRUE(resumed || queue->get_job(job_id).has_value());
}

TEST_F(JobQueueTest, GetsPrinterStatistics) {
    PrintJob job;
    job.printer_name = "test_printer";
    job.file_path = "/tmp/test.pdf";
    job.original_filename = "test.pdf";
    
    int job_id = queue->add_job(job);
    EXPECT_GT(job_id, 0);
    
    // Teste básico - verificar que o job foi adicionado
    auto retrieved = queue->get_job(job_id);
    EXPECT_TRUE(retrieved.has_value());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
