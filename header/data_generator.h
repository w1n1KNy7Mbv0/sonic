#ifndef _DATA_GENERATOR_H_
#define _DATA_GENERATOR_H_

#include <array>
#include <climits>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <valarray>

#include "helper_functions.h"

#define ROW_SIZE 67108864
#define COLUMN_SIZE 4

inline bool fileExists(const std::string& name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}

namespace datagenerator {
template <typename... ColumnTypes> class Table {
public:
  template <typename ColumnType = int32_t, size_t Length = 4>
  static std::unique_ptr<std::valarray<std::array<ColumnType, Length>>>
  generateTable(const std::string distribution = "random", const size_t rows = ROW_SIZE) {

    typedef std::array<ColumnType, Length> Row;

    auto table = std::make_unique<std::valarray<Row>>(rows);

    srand(time(0));

    std::random_device rd;
    std::mt19937 gen(rd());

    if(distribution == "random") {
      for(auto i = 0; i < rows; i++)
        for(auto j = 0; j < Length; j++)
          (*table)[i][j] = (ColumnType)(rand());
    } else if(distribution == "possion") {
      std::poisson_distribution<> dis(4);
      for(auto i = 0; i < rows; i++)
        for(auto j = 0; j < Length; j++)
          (*table)[i][j] = (ColumnType)(dis(gen));
    } else if(distribution == "gamma") {
      std::gamma_distribution<> dis(1, 2);
      for(auto i = 0; i < rows; i++)
        for(auto j = 0; j < Length; j++)
          (*table)[i][j] = (ColumnType)(dis(gen));
    } else if(distribution == "uniform") {
      std::uniform_real_distribution<> dis(1, rows);
      for(auto i = 0; i < rows; i++)
        for(auto j = 0; j < Length; j++)
          (*table)[i][j] = (ColumnType)(dis(gen));
    } else if(distribution == "worst-case") {
      for(auto i = 0; i < rows; i++) {
        for(auto j = 0; j < Length - 1; j++)
          (*table)[i][j] = 1;
        (*table)[i][Length - 1] = rand();
      }
    }

    return table;
  }

  template <typename Schema = std::tuple<ColumnTypes...>>
  static std::vector<Schema> readFromFile(std::string file_path, size_t table_size) {
    std::vector<Schema> table;
    if(!fileExists(file_path)) {
      return table;
    }

    std::ifstream input_file(file_path);
    std::string line, word;

    for(auto i = 0; i < table_size; i++) {
      getline(input_file, line);
      std::stringstream line_stream(line);

      std::vector<std::string> columns_vec;
      auto elementCounter = 0;
      while(getline(line_stream, word, ',')) {
        columns_vec.emplace_back(word);
        elementCounter++;
      }
      while(elementCounter < sizeof...(ColumnTypes)) {
        columns_vec.emplace_back("");
        elementCounter++;
      }
      if(columns_vec.size() > 0) {
        table.emplace_back(
            vectorToTuple<sizeof...(ColumnTypes), std::string, std::tuple<ColumnTypes...>>(
                columns_vec));
      }
    }

    return table;
  }

  template <typename OutputSchema, typename InputSchema>
  static vector<OutputSchema> readFromFileToInt(std::string file_path, size_t table_size) {
    std::vector<InputSchema> table;
    vector<OutputSchema> tableInt;
    if(!fileExists(file_path)) {
      return tableInt;
    }

    std::ifstream input_file(file_path);
    std::string line, word;

    for(auto i = 0; i < table_size; i++) {
      getline(input_file, line);
      std::stringstream line_stream(line);

      std::vector<std::string> columns_vec;
      auto elementCounter = 0;
      while(getline(line_stream, word, ',')) {
        columns_vec.emplace_back(word);
        elementCounter++;
      }
      while(elementCounter < sizeof...(ColumnTypes)) {
        columns_vec.emplace_back("");
        elementCounter++;
      }
      if(columns_vec.size() > 0) {
        table.emplace_back(
            vectorToTuple<sizeof...(ColumnTypes), std::string, std::tuple<ColumnTypes...>>(
                columns_vec));
      }
    }

    vector<unordered_map<string, int>> dictionaries;
    for(auto i = 0; i < sizeof...(ColumnTypes); i++) {
      auto umap = unordered_map<string, int>();
      dictionaries.emplace_back(umap);
    }
    for(auto i = 0; i < table.size(); i++) {
      vector<int> rowInt;
      auto j = 0;
      for_each(table[i], [&](auto const& e) {
        auto value = 0;
        try {
          value = (dictionaries[j]).at(toString(e));
        } catch(...) {
          (dictionaries[j]).insert(make_pair(toString(e), i));
          value = i;
        }
        j++;
        rowInt.emplace_back(value);
      });
      tableInt.emplace_back(vectorToTuple<tuple_size_v<OutputSchema>, int, OutputSchema>(rowInt));
    }

    return tableInt;
  }

  template <typename Schema = std::tuple<ColumnTypes...>>
  static std::vector<Schema> generateIntegerTuples(const std::string distribution = "random",
                                                   const size_t rows = ROW_SIZE) {

    std::vector<Schema> table;
    table.reserve(rows);

    array<int, sizeof...(ColumnTypes)> tmp;

    for(auto i = 0; i < rows; i++) {
      if(distribution == "random") {
        for(auto i = 0; i < sizeof...(ColumnTypes); i++) {
          tmp[i] = rand();
        }
      } else if(distribution == "worst-case") {
        for(auto i = 0; i < sizeof...(ColumnTypes); i++) {
          tmp[i] = rand() % 10;
        }
      } else if(distribution == "normal") {
        std::random_device rd{};
        std::mt19937 gen{rd()};
        auto constexpr mean = 8388608;
        auto constexpr stdv = 3;
        std::normal_distribution<> d{mean, stdv};
        for(auto i = 0; i < sizeof...(ColumnTypes); i++) {
          tmp[i] = std::round(d(gen));
        }
      } else if(distribution == "join") {
        for(auto i = 0; i < sizeof...(ColumnTypes); i++) {
          tmp[i] = rand() % (int)(0.9995 * (double)rows);
        }
      }
      table.emplace_back(arrayToTuple<ColumnTypes...>(tmp));
    }

    return table;
  }

  template <typename Schema = std::tuple<ColumnTypes...>>
  static std::vector<Schema> generateStringTuples(const std::string distribution = "random",
                                                  const size_t rows = ROW_SIZE) {

    std::vector<Schema> table;
    table.reserve(rows);

    vector<string> tmp;

    for(auto i = 0; i < rows; i++) {
      if(distribution == "random") {
        for(auto i = 0; i < sizeof...(ColumnTypes); i++) {
          string s = "";
          s += rand();
          tmp.emplace_back(s);
        }
      } else if(distribution == "worst-case") {
        auto n = rand() % 4096;
        string s = "";
        s += n;
        for(auto i = 0; i < sizeof...(ColumnTypes) - 1; i++) {
          tmp.emplace_back(s);
        }
        s = "";
        s += rand();
        tmp.emplace_back(s);
      } else if(distribution == "normal") {
        std::random_device rd{};
        std::mt19937 gen{rd()};
        auto constexpr mean = 8388608;
        auto constexpr stdv = 3;
        std::normal_distribution<> d{mean, stdv};
        for(auto i = 0; i < sizeof...(ColumnTypes); i++) {
          string s = "";
          s += std::round(d(gen));
          tmp.emplace_back(s);
        }
      } else if(distribution == "join") {
        for(auto i = 0; i < sizeof...(ColumnTypes); i++) {
          string s = "";
          s += rand() % (int)(0.95 * (double)rows);
          tmp.emplace_back(s);
        }
      }
      table.emplace_back(vectorToTuple<sizeof...(ColumnTypes), string, tuple<ColumnTypes...>>(tmp));
    }

    return table;
  }
};
} // namespace datagenerator

#endif
