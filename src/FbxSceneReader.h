#pragma once

#include <iosfwd>
#include <string>

// Loads an FBX scene with ufbx and writes a human-readable summary.
class FbxSceneReader {
public:
    bool printSummary(const std::string& filename, std::ostream& output, std::ostream& errors) const;
};
