//
// Created by Lukas Varhol on 11/7/2025.
//
#include <ctime>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <numeric>
#include "../include/logging/logger.h"

// child logger class
class TestableLogger : public Logger {
public:
  static TestableLogger &get_instance() {
    static TestableLogger instance;
    return instance;
  }

  void initialize_for_test(const std::filesystem::path &log_dir) {
    base_dir_ = log_dir; // base_dir_ is protected member of Logger Class
    std::filesystem::create_directories(base_dir_);
    clear_cache();
  }

  void set_mock_date(const std::string &date) {
    mock_date_ = date;
    clear_cache();
  }

  void clear_mocks() {
    mock_date_.clear();
    clear_cache();
  }

  void clear_cache() {
    std::lock_guard<std::mutex> lock(cache_mutex_);
    cached_date_.clear();
    cached_date_dir_.clear();
  }

  void reset_for_testing() {
    clear_mocks();
    clear_cache();
    base_dir_.clear();
  }

protected:
  std::string get_current_date() const override {
    if (!mock_date_.empty()) {
      return mock_date_;
    }
    return Logger::get_current_date();
  }

private:
  TestableLogger() = default;
  std::string mock_date_;
};

#define TEST_LOG TestableLogger::get_instance()


// test fixture
class LoggerTest : public ::testing::Test {
protected:
  std::filesystem::path test_dir;

  void SetUp() override {

    test_dir = std::filesystem::temp_directory_path() / ("logger_test_");
    std::filesystem::create_directory(test_dir);
    TEST_LOG.reset_for_testing();
    TEST_LOG.initialize_for_test(test_dir);
  }
  void TearDown() override {
    TEST_LOG.reset_for_testing();
    std::filesystem::remove_all(test_dir);
  }

  void create_old_log_file(const std::string &date_dir, const std::string &file_name, int days_old) {
    std::filesystem::path file_path = test_dir / date_dir / file_name;
    std::filesystem::create_directory(file_path.parent_path());
    std::ofstream file(file_path.string());
    if (file.is_open()) {
      file << "Log entry created for testing" << std::endl;
      file << "Date directory: " << date_dir << std::endl;
      file << "Days old: " << days_old << std::endl;
      file.close();
    }

    auto now = std::filesystem::file_time_type::clock::now();
    auto old_time = now - std::chrono::hours(24 * days_old);
    std::filesystem::last_write_time(file_path, old_time);
  }

  std::string read_file_content(const std::string &relative_path) {
    std::ifstream file(test_dir / relative_path);
    if (!file.is_open()) {
      return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  std::vector<std::string> read_file_lines(const std::string &relative_path) {
    std::vector<std::string> lines;
    std::ifstream file(test_dir / relative_path);

    if (!file.is_open()) {
      return lines;
    }

    std::string line;
    while (std::getline(file, line)) {
      lines.push_back(line);
    }

    return lines;
  }
};
// ==============================================================================
// Tests
// ==============================================================================

// FILE CREATION & DESTRUCTION

TEST_F(LoggerTest, FilesCreatedOnStart) {
  std::string date = "2025-01-01";
  TEST_LOG.set_mock_date(date);

  std::filesystem::path trades_file = test_dir / date / "trades.log";
  std::filesystem::path orders_file = test_dir / date / "orders.log";
  std::filesystem::path signals_file = test_dir / date / "signals.log";
  std::filesystem::path system_file = test_dir / date / "system.log";
  std::filesystem::path debug_file = test_dir / date / "debug.log";

  EXPECT_TRUE(std::filesystem::exists(trades_file));
  EXPECT_TRUE(std::filesystem::exists(orders_file));
  EXPECT_TRUE(std::filesystem::exists(signals_file));
  EXPECT_TRUE(std::filesystem::exists(system_file));
  EXPECT_TRUE(std::filesystem::exists(debug_file));
}

TEST_F(LoggerTest, CreatesNewDirectoryOnDateChange) {
  std::string day1 = "2025-01-01";
  std::string day2 = "2025-01-02";

  TEST_LOG.set_mock_date(day1);
  TEST_LOG.info(TRADES, "Today is 2025-01-01");

  TEST_LOG.set_mock_date(day2);
  TEST_LOG.info(TRADES, "Today is 2025-01-02");

  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2025-01-01/trades.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2025-01-02/trades.log"));
}

TEST_F(LoggerTest, SingleMessageCreatesLog) {
  TEST_LOG.set_mock_date("2025-01-01");

  TEST_LOG.info(TRADES, "Dummy");
  ASSERT_EQ(std::filesystem::exists(test_dir.string() + "/2025-01-01/dummy.log"), true);
}

TEST_F(LoggerTest, MultipleLogMessagesAppended) {
  TEST_LOG.set_mock_date("2025-01-01");

  TEST_LOG.info(TRADES, "Dummy");
  TEST_LOG.info(TRADES, "Dummy2");

  std::ifstream file(test_dir.string() + "/2025-01-01/trades.log");
  int count = 0;
  std::string line;
  while (std::getline(file, line)) {
    count++;
  }
  file.close();

  ASSERT_EQ(count, 2);
}

TEST_F(LoggerTest, OldLogsDeleted) {
  const std::string base_date = "2025-01-01";

  // Create old files that should be deleted
  create_old_log_file("2024-12-20", "debug.log", 12); // > 7 days
  create_old_log_file("2024-10-01", "system.log", 92); // > 90 days
  create_old_log_file("2024-01-01", "signals.log", 365); // > 1 year
  create_old_log_file("2022-01-01", "orders.log", 1095); // > 3 years

  // Create recent files that should be kept
  create_old_log_file("2024-12-28", "debug.log", 4); // < 7 days
  create_old_log_file("2024-11-01", "system.log", 61); // < 90 days
  create_old_log_file("2024-06-01", "signals.log", 214); // < 1 year
  create_old_log_file("2023-01-01", "orders.log", 730); // < 3 years
  create_old_log_file("2020-01-01", "trades.log", 1826); // Forever retention

  TEST_LOG.set_mock_date(base_date);

  EXPECT_FALSE(std::filesystem::exists(test_dir.string() + "/2024-12-20/debug.log"));
  EXPECT_FALSE(std::filesystem::exists(test_dir.string() + "/2024-10-01/system.log"));
  EXPECT_FALSE(std::filesystem::exists(test_dir.string() + "/2024-01-01/signals.log"));
  EXPECT_FALSE(std::filesystem::exists(test_dir.string() + "/2022-01-01/orders.log"));

  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2024-12-28/debug.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2024-11-01/system.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2024-06-01/signals.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2023-01-01/orders.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2020-01-01/trades.log"));
}


// DATA INTEGRITY

TEST_F(LoggerTest, InfoWithCorrectFormat) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.info(TRADES, "Test message");

  std::string content = read_file_content(test_dir.string() + "/2025-01-01/trades.log");

  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
  EXPECT_TRUE(content.find("Test message") != std::string::npos);
}

TEST_F(LoggerTest, WarnWithCorrectFormat) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.warn(TRADES, "Test message");

  std::string content = read_file_content(test_dir.string() + "/2025-01-01/trades.log");

  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[WARN]") != std::string::npos);
  EXPECT_TRUE(content.find("Test message") != std::string::npos);
}

TEST_F(LoggerTest, ErrorWithCorrectFormat) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.error(TRADES, "Test message");

  std::string content = read_file_content(test_dir.string() + "/2025-01-01/trades.log");

  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[ERROR]") != std::string::npos);
  EXPECT_TRUE(content.find("Test message") != std::string::npos);
}

TEST_F(LoggerTest, DebugWithCorrectFormat) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.debug(TRADES, "Test message");

  std::string content = read_file_content(test_dir.string() + "/2025-01-01/trades.log");

  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[DEBUG]") != std::string::npos);
  EXPECT_TRUE(content.find("Test message") != std::string::npos);
}


TEST_F(LoggerTest, EmptyMessage) {
  TEST_LOG.set_mock_date("2025-01-01");

  TEST_LOG.info(TRADES, "");

  std::string content = read_file_content(test_dir.string() + "/2025-01-01/trades.log");

  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
}

TEST_F(LoggerTest, VeryLongMessage) {
  TEST_LOG.set_mock_date("2025-01-01");

  std::string long_message(10000, 'A');
  long_message += " END_MARKER";

  TEST_LOG.info(TRADES, long_message);

  std::string content = read_file_content(test_dir.string() + "/2025-01-01/trades.log");
  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
  EXPECT_TRUE(content.find("END_MARKER") != std::string::npos);
}

TEST_F(LoggerTest, SpecialCharacters) {
  TEST_LOG.set_mock_date("2025-01-01");

  std::string special_message = R"(@$#^)!()<,./*#(Q\123/123yo12uy3()83{}[])";
  TEST_LOG.info(TRADES, special_message);

  std::string content = read_file_content(test_dir.string() + "/2025-01-01/trades.log");

  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
  EXPECT_TRUE(content.find("@$#^") != std::string::npos); // Part of the message
  EXPECT_TRUE(content.find("yo12uy3") != std::string::npos); // Another part
}

// THREADNG BEHAVIOUR

TEST_F(LoggerTest, NotOnMainThread) {
  TEST_LOG.set_mock_date("2025-01-01");

  std::thread::id main_thread_id = std::this_thread::get_id();
  std::thread::id logger_thread_id;

  TEST_LOG.info(TRADES, "Test message");
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  logger_thread_id =
      TEST_LOG
          .background_thread_id(); // TODO: create this method to access private member variable background_thread_id_.

  EXPECT_NE(main_thread_id, logger_thread_id) << "Logger should run on a different thread";
}

TEST_F(LoggerTest, MultipleLogCallsNonBlocking) {
  TEST_LOG.set_mock_date("2025-01-01");

  std::atomic<int> completed_calls{0};
  std::vector<std::chrono::milliseconds> call_durations;
  const int NUM_THREADS = 4;
  const int CALLS_PER_THREAD = 1000;

  std::vector<std::thread> threads;
  std::mutex duration_mutex;

  for (int i = 0; i < NUM_THREADS; ++i) {
    threads.emplace_back([&, i]() {
      for (int j = 0; j < CALLS_PER_THREAD; ++j) {
        auto start = std::chrono::high_resolution_clock::now();
        TEST_LOG.info(TRADES, "Thread " + std::to_string(i) + " message " + std::to_string(j));
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        {
          std::lock_guard<std::mutex> lock(duration_mutex);
          call_durations.push_back(duration);
        }
        completed_calls++;
      }
    });
  }
  for (auto &thread: threads) {
    thread.join();
  }

  EXPECT_EQ(completed_calls.load(), NUM_THREADS * CALLS_PER_THREAD);
  auto max_duration = *std::max_element(call_durations.begin(), call_durations.end());
  EXPECT_LT(max_duration.count(), 1000) << "SOme log calls blocked for too long";
}

TEST_F(LoggerTest, RunAfterMainCrash) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.info(TRADES, "Before Crash");

  try {
    TEST_LOG.info(TRADES, "Durung crash simulation");
    throw std::runtime_error("simulated crash");
  } catch (const std::exception &) {
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  EXPECT_NO_THROW(TEST_LOG.info(TRADES, "after crash"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2025-01-01/trades.log"));
}

// QUEUE/BUFFER BEHAVIOUR

TEST_F(LoggerTest, MessagesQueuedWithRapidLogging) {
  TEST_LOG.set_mock_date("2025-01-01");
  const int RAPID_MESSAGES = 10000;

  auto start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < RAPID_MESSAGES; ++i) {
    TEST_LOG.info(TRADES, "Rapid message " + std::to_string(i));
  }
  auto end = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  EXPECT_LT(duration.count(), 1000) << "Rapid messages blocked for too long";

  size_t queue_size = TEST_LOG.queue_size(); // TODO: create this method to access private member variable
  EXPECT_GT(queue_size, 0) << "Messsages should be queued during rapid logging";

  std::cout << "Queue size: " << queue_size << std::endl;
}

TEST_F(LoggerTest, QueueEmptiesAsBackgroundThreadProcess) {
  TEST_LOG.set_mock_date("2025-01-01");

  // fill queue with messages
  for (int i = 0; i < 1000; i++) {
    TEST_LOG.info(TRADES, "Queue test " + std::to_string(i));
  }

  size_t initial_queue_size = TEST_LOG.queue_size();
  EXPECT_GT(initial_queue_size, 0);

  auto start_time = std::chrono::high_resolution_clock::now();
  const auto timeout = std::chrono::seconds(5);

  while (TEST_LOG.queue_size() > 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto elapsed = std::chrono::high_resolution_clock::now() - start_time;
  }

  EXPECT_EQ(TEST_LOG.queue_size(), 0);

  auto lines = read_file_lines(test_dir.string() + "/2025-01-01/trades.log");
  EXPECT_EQ(lines.size(), 1000);
}

TEST_F(LoggerTest, QueueHandlesOverflow) {
  TEST_LOG.set_mock_date("2025-01-01");
  const int OVERFLOW_MESSAGES = 100000; // attempt to overwhelm queue

  EXPECT_NO_THROW({
    for (int i = 0; i < OVERFLOW_MESSAGES; ++i) {
      TEST_LOG.info(TRADES, "Over flow test " + std::to_string(i));
    }
  });

  EXPECT_NO_THROW(TEST_LOG.info(TRADES, "After overflow"));
  std::this_thread::sleep_for(std::chrono::seconds(2));


  EXPECT_TRUE(std::filesystem::exists(test_dir.string()+"2025-01-01/trades.log"));
  auto lines = read_file_lines(test_dir.string()+"2025-01-01/trades.log");
  EXPECT_GT(lines.size(), 1000);
}

TEST_F(LoggerTest, QueueSizeCanBeMonitored) {
  TEST_LOG.set_mock_date("2025-01-01");
  EXPECT_EQ(TEST_LOG.queue_size(), 0);

  TEST_LOG.info(TRADES, "Message 1");
  TEST_LOG.warn(TRADES, "Message 2");
  TEST_LOG.error(TRADES, "Message 3");
  TEST_LOG.debug(TRADES, "Message 4");

  size_t queue_size_after_adding = TEST_LOG.queue_size();
  EXPECT_GE(queue_size_after_adding, 0);
  std::vector<size_t> queue_sizes;
  for (int i = 0; i < 100; ++i) {
    queue_sizes.push_back(TEST_LOG.queue_size());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  EXPECT_NO_THROW(TEST_LOG.get_queue_size());

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  EXPECT_EQ(TEST_LOG.queue_size(), 0);
}


// RESOURCE MANAGEMENT

TEST_F(LoggerTest, CleanUpOnDestruction) {
  std::thread::id background_thread_id;

  {
    TEST_LOG.set_mock_date("2025-01-01");
    for (int i = 0; i < 1000; ++i) {
      TEST_LOG.info(TRADES, "Clean up test " + std::to_string(i));
    }
    background_thread_id = TEST_LOG.background_thread_id();
  }

  // logger should be destroyed here

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(std::filesystem::exists(test_dir.string()+"2025-01-01/trades.log"));
}

TEST_F(LoggerTest, MemoryLeakAfterHeavyUsage) {
  auto testLoggerCycle = [this]() {

    TEST_LOG.setMockDate("2025-01-01");

    for (int i = 0; i < 1000; ++i) {
      TEST_LOG.info(TRADES, "Cycle test " + std::to_string(i));
    }

    // Give time for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Logger destroyed when function exits
  };

  // Run many cycles - if there are memory leaks, this might cause issues
  auto start = std::chrono::steady_clock::now();

  for (int cycle = 0; cycle < 100; ++cycle) {
    testLoggerCycle();
  }

  auto end = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  // Should complete without hanging or crashing
  EXPECT_LT(duration.count(), 30000) << "Test took too long - possible memory issues";

  std::cout << "100 logger cycles completed in " << duration.count() << " ms" << std::endl;
}


TEST_F(LoggerTest, BackgroundThreadTerminated) {
  std::thread::id background_thread_id;
  bool thread_was_running = false;

  {
    TEST_LOG.setMockDate("2025-01-01");

    TEST_LOG.info(TRADES, "Thread test");

    // Give time for thread to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (TEST_LOG.has_background_thread()) { // TODO: implement this function in Logger class
      background_thread_id = TEST_LOG.background_thread_id();
      thread_was_running = true;
    }

    // Logger destroyed here
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  EXPECT_TRUE(thread_was_running); // Verify thread was actually running
}

// CACHING BEHAVIOUR

TEST_F(LoggerTest, DateCalculatedOnlyOncePerDay) {
  // Start with no mock date to use real date calculation
  TEST_LOG.clear_mocks();

  const int NUM_MESSAGES = 100;
  std::vector<std::chrono::microseconds> call_times;

  for (int i = 0; i < NUM_MESSAGES; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    TEST_LOG.info(TRADES, "Message " + std::to_string(i));
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    call_times.push_back(duration);
  }

  // First call should be slower (date calculation + cache population)
  // Subsequent calls should be faster (using cached date)
  auto first_call_time = call_times[0];
  auto avg_subsequent_time = std::accumulate(call_times.begin() + 10, call_times.end(),
                                           std::chrono::microseconds(0)) / (call_times.size() - 10);

  // Later calls should generally be faster due to caching
  EXPECT_LT(avg_subsequent_time.count(), first_call_time.count() * 1.5)
    << "Subsequent calls should be faster due to date caching";

  std::cout << "First call: " << first_call_time.count() << "μs, "
            << "Avg subsequent: " << avg_subsequent_time.count() << "μs" << std::endl;
}



int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
