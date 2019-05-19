#include "mull/Reporters/MutationTestingElementsReporter.h"

#include "mull/Config/RawConfig.h"
#include "mull/Logger.h"
#include "mull/MutationResult.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Reporters/SourceManager.h"
#include "mull/Result.h"

#include "json11/json11.hpp"

#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace mull;
using namespace json11;

static json11::Json
createFiles(const Result &result,
            const std::set<MutationPoint *> &killedMutants) {
  SourceManager sourceManager;

  Json::object filesJSON;

  // Mutation Testing Elements Schema suggests storing mutation points based
  // on their source file.
  // Step 1: we create a map: "file => all of its mutation points".
  std::map<std::string, std::vector<MutationPoint *>> mutationPointsPerFile;
  for (auto &mutationResult : result.getMutationResults()) {
    MutationPoint *mutant = mutationResult->getMutationPoint();

    auto &sourceLocation = mutant->getSourceLocation();
    auto sourceCodeLine = sourceManager.getLine(sourceLocation);
    assert(sourceLocation.column < sourceCodeLine.size());

    if (filesJSON.count(sourceLocation.filePath) == 0) {
      std::pair<std::string, std::vector<MutationPoint *>> pair =
          std::make_pair(sourceLocation.filePath,
                         std::vector<MutationPoint *>());

      mutationPointsPerFile.insert(pair);
    }

    std::vector<MutationPoint *> &currentFileMutationPoints =
        mutationPointsPerFile.at(sourceLocation.filePath);
    currentFileMutationPoints.push_back(mutant);
  }

  // Step 2: Iterate through each file and dump the information about each
  // mutation points to its file's JSON entry.
  for (const std::pair<std::string, std::vector<MutationPoint *>>
           &fileMutationPoints : mutationPointsPerFile) {
    Json::object fileJSON;
    fileJSON["language"] = "cpp";

    // Getting source code
    std::ifstream fileInputStream(fileMutationPoints.first);
    std::stringstream sourceCodeBuffer;
    sourceCodeBuffer << fileInputStream.rdbuf();
    fileJSON["source"] = sourceCodeBuffer.str();

    Json::array mutantsEntries;

    for (MutationPoint *mutationPoint : fileMutationPoints.second) {
      auto &sourceLocation = mutationPoint->getSourceLocation();

      std::string status =
          (killedMutants.find(mutationPoint) == killedMutants.end())
              ? "Survived"
              : "Killed";

      Json mpJson = Json::object{
          {"id", mutationPoint->getMutator()->getUniqueIdentifier()},
          {"mutatorName", mutationPoint->getDiagnostics()},
          {"replacement", "-"},
          {"location",
           Json::object{
               {"start", Json::object{{"line", sourceLocation.line},
                                      {"column", sourceLocation.column}}},
               {"end", Json::object{{"line", sourceLocation.line},
                                    {"column", sourceLocation.column}}}}},
          {"status", status}};
      mutantsEntries.push_back(mpJson);
    }

    fileJSON["mutants"] = mutantsEntries;
    filesJSON[fileMutationPoints.first] = fileJSON;
  }

  return filesJSON;
}

void MutationTestingElementsReporter::reportResults(const Result &result,
                                                    const RawConfig &config,
                                                    const Metrics &metrics) {
  if (result.getMutationPoints().empty()) {
    Logger::info() << "No mutants found. Mutation score: infinitely high\n";
    return;
  }

  std::set<MutationPoint *> killedMutants;
  for (auto &mutationResult : result.getMutationResults()) {
    if (!mutationResult->hasMutantSurvived()) {
      auto mutant = mutationResult->getMutationPoint();

      killedMutants.insert(mutant);
    }
  }

  auto rawScore =
      double(killedMutants.size()) / double(result.getMutationPoints().size());
  auto score = uint(rawScore * 100);

  Json my_json = Json::object{
      {"mutationScore", (int)score},
      {"thresholds", Json::object{{"high", 80}, {"low", 60}}},
      {"files", createFiles(result, killedMutants)},
  };
  std::string json_str = my_json.dump();
  Logger::info() << json_str << "\n";

  std::string outPath = "/tmp/mull.mutation-testing-elements.json";
  const std::string &configOutPath =
      config.getMutationTestingElementsReportPath();

  if (!configOutPath.empty()) {
    outPath = configOutPath;
  } else {
    Logger::info()
        << "MutationTestingElementsReporter: the output path is not provided, "
        << "using default output path: "
        << "\n"
        << outPath << "\n";
  }

  std::ofstream out(outPath);
  out << json_str;
  out.close();
}
