#pragma once

#include <string>

struct Arguments
{
    int width;
    int height;
    int quality;

    std::string rFormula;
    std::string gFormula;
    std::string bFormula;

    std::string output;

    Arguments()
    {
        width = 0;
        height = 0;
        quality = 100;
    }
};

bool ParseArguments(
    int argc,
    char* argv[],
    Arguments& args);

void PrintUsage();