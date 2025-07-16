//
// Created by Lukas Varhol on 11/7/2025.
//
#include "../include/logging/logger.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

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
    base_directory_.clear();
  }

protected:
  std::string get_current_date() const override {
    if (!mock_date_.empty()) {
      return mock_date_;
    }
    return Logger::getCurrentDate();
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

  std::string read_file_content(const std::string& relative_path) {
    std::ifstream file(test_dir / relative_path);
    if (!file.is_open()) {
      return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  }

  std::vector<std::string> read_file_lines(const std::string& relative_path) {
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

// File Creation and Destruction

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

  // ASSERT - Old files deleted
  EXPECT_FALSE(std::filesystem::exists(test_dir.string() + "/2024-12-20/debug.log"));
  EXPECT_FALSE(std::filesystem::exists(test_dir.string() + "/2024-10-01/system.log"));
  EXPECT_FALSE(std::filesystem::exists(test_dir.string() + "/2024-01-01/signals.log"));
  EXPECT_FALSE(std::filesystem::exists(test_dir.string() + "/2022-01-01/orders.log"));

  // ASSERT - Recent files kept
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2024-12-28/debug.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2024-11-01/system.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2024-06-01/signals.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2023-01-01/orders.log"));
  EXPECT_TRUE(std::filesystem::exists(test_dir.string() + "/2020-01-01/trades.log"));
}


// Data Integrity

TEST_F(LoggerTest, InfoWithCorrectFormat) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.info(TRADES, "Test message");

  std::string content = read_file_content(test_dir.string()+"/2025-01-01/trades.log");
  
  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
  EXPECT_TRUE(content.find("Test message") != std::string::npos);
}

TEST_F(LoggerTest, WarnWithCorrectFormat) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.warn(TRADES, "Test message");
  
  std::string content = read_file_content(test_dir.string()+"/2025-01-01/trades.log");
  
  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[WARN]") != std::string::npos);
  EXPECT_TRUE(content.find("Test message") != std::string::npos);
}

TEST_F(LoggerTest, ErrorWithCorrectFormat) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.error(TRADES, "Test message");

  std::string content = read_file_content(test_dir.string()+"/2025-01-01/trades.log");
  
  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[ERROR]") != std::string::npos);
  EXPECT_TRUE(content.find("Test message") != std::string::npos);
}

TEST_F(LoggerTest, DebugWithCorrectFormat) {
  TEST_LOG.set_mock_date("2025-01-01");
  TEST_LOG.debug(TRADES, "Test message");

  std::string content = read_file_content(test_dir.string()+"/2025-01-01/trades.log");
  
  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[DEBUG]") != std::string::npos);
  EXPECT_TRUE(content.find("Test message") != std::string::npos);
}


TEST_F(LoggerTest, EmptyMessage) {
  TEST_LOG.set_mock_date("2025-01-01");
    
  TEST_LOG.info(TRADES, "");
    
  std::string content = read_file_content(test_dir.string()+"/2025-01-01/trades.log");
  
  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
}

TEST_F(LoggerTest, VeryLongMessage) {
  TEST_LOG.set_mock_date("2025-01-01");

  std::string long_message(10000, 'A');
  long_message += " END_MARKER";

  TEST_LOG.info(TRADES, long_message);

  std::string content = read_file_content(test_dir.string()+"/2025-01-01/trades.log");
  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
  EXPECT_TRUE(content.find("END_MARKER") != std::string::npos);
}

TEST_F(LoggerTest, SpecialCharacters) {
  TEST_LOG.set_mock_date("2025-01-01");

  std::string special_message = R"(@$#^)!()<,./*#(Q\123/123yo12uy3()83{}[])";
  TEST_LOG.info(TRADES, special_message);

  std::string content = read_file_content(test_dir.string()+"/2025-01-01/trades.log");

  EXPECT_TRUE(content.find("2025-01-01") != std::string::npos);
  EXPECT_TRUE(content.find("[INFO]") != std::string::npos);
  EXPECT_TRUE(content.find("@$#^") != std::string::npos);  // Part of the message
  EXPECT_TRUE(content.find("yo12uy3") != std::string::npos); // Another part
}

// Threading Behaviour

TEST_F(LoggerTest, NotOnMainThread) {}

TEST_F(LoggerTest, ImmediateReturn) {}

TEST_F(LoggerTest, MultipleLogCallsNonBlocking) {}

TEST_F(LoggerTest, RunAfterMainCrash) {}

// Queue/Buffer Behaviour

TEST_F(LoggerTest, MessagesQueuedWithRapidLogging) {}

TEST_F(LoggerTest, QueueEmptiesAsBackgroundThreadProcess) {}

TEST_F(LoggerTest, QueueHandlesOverflow) {}

TEST_F(LoggerTest, QueueSizeCanBeMonitored) {}

// Error Handling

TEST_F(LoggerTest, InvalidFilepath) {}

TEST_F(LoggerTest, ReadOnlyFile) {}

TEST_F(LoggerTest, DiskFull) {}

TEST_F(LoggerTest, DirectoryDoesNotExist) {}

// Resource Management

TEST_F(LoggerTest, CleanUpOnDestruction) {}

TEST_F(LoggerTest, MemoryLeakAfterHeavyUsage) {}

TEST_F(LoggerTest, FileClosed) {}

TEST_F(LoggerTest, BackgroundThreadTerminated) {}


int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
