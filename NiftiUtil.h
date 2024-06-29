#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>

#include "NiftiHeader.h"

int unitSize(int code) {
    switch (code)
    {
    case 1: return sizeof(bool);
    case 2: return sizeof(unsigned char);
    case 4: return sizeof(signed short);
    case 8: return sizeof(signed int);
    case 16: return sizeof(float);
    case 64: return sizeof(double);
    case 256: return sizeof(signed char);
    case 512: return sizeof(unsigned short);
    case 768: return sizeof(unsigned int);
    case 1024: return sizeof(long long);
    case 1280: return sizeof(unsigned long long);
    case 1536: return sizeof(long double);
    default: throw std::exception("Invalid Code");
    }
}

struct NiftiFile {
	nifti_1_header header;
	std::vector<uint8_t> data;
};

struct NiftiFile4D {
    nifti_1_header header;
    std::vector<std::vector<uint8_t>> data;
};

template <typename LoadedType>
NiftiFile _loadNifti(const std::string& filepath) {
	std::ifstream file(filepath, std::ios::binary);

	if (!file.is_open())
		throw std::exception("Error: Unable to open Nifti file");

    nifti_1_header header;
    file.read(reinterpret_cast<char*>(&header), sizeof(nifti_1_header));
    

    if (strncmp(header.magic, "n+1\0", 4) != 0) {
        throw std::exception("Error: Invalid Nifti File");
    }


    size_t dataSize = header.datatype;

    for (int i = 0; i < header.dim[0]; ++i)
        dataSize *= header.dim[i + 1];

    file.seekg(header.vox_offset);

    std::vector<LoadedType> data(dataSize / sizeof(LoadedType));
    file.read(reinterpret_cast<char*>(data.data()), dataSize);

    file.close();

    std::vector<uint8_t> asUint;

    std::cout << typeid(LoadedType).name();
   
    LoadedType max = *std::max_element(data.begin(), data.end());
    LoadedType min = *std::min_element(data.begin(), data.end());
    for (const LoadedType& i : data)
        asUint.push_back(uint8_t((i - min) / (max - min) * 255));

    std::cout << "DONE\n";
    return {header, asUint};
}

NiftiFile load4d(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open())
        throw std::exception("Error: Unable to open Nifti file");

    nifti_1_header header;
    file.read(reinterpret_cast<char*>(&header), sizeof(nifti_1_header));

    std::cout << header.dim << std::endl;
}

nifti_1_header getHeader(const char* filename) {
    // Open the file in binary mode
    std::ifstream file(filename, std::ios::binary);

    // Check if the file is opened successfully
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read the NIfTI header
    nifti_1_header header;
    file.read(reinterpret_cast<char*>(&header), sizeof(nifti_1_header));
    file.close();
    return header;
}

NiftiFile loadNifti(const std::string& filepath) {

    auto header = getHeader(filepath.c_str());

    switch (header.datatype)
    {
    case 2: return _loadNifti<unsigned char>(filepath);
    case 4: return _loadNifti<signed short>(filepath);;
    case 8: return _loadNifti<signed int>(filepath);;
    case 16: return _loadNifti<float>(filepath);;
    case 64: return _loadNifti<double>(filepath);
    case 256: return _loadNifti<signed char>(filepath);
    case 512: return _loadNifti<unsigned short>(filepath);
    case 768: return _loadNifti<unsigned int>(filepath);;
    case 1024: return _loadNifti<long long>(filepath);
    case 1280: return _loadNifti<unsigned long long>(filepath);
    case 1536: return _loadNifti<long double>(filepath);;
    default: throw std::exception("Invalid Code");
    }
}



// Function to load 4D NIfTI file into a uint8_t array
NiftiFile4D loadNifti4d(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return { {}, {} };
    }

    nifti_1_header header;
    file.read(reinterpret_cast<char*>(&header), sizeof(nifti_1_header));


    std::cout << "dims: ";
    for (auto i : header.dim) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;

    std::cout << "dtype: " << header.datatype << std::endl;
    std::cout << "vox: " << header.vox_offset << std::endl;

    size_t dataSize = sizeof(float);

    for (int i = 0; i < header.dim[0]; ++i)
        dataSize *= header.dim[i + 1];

    file.seekg(header.vox_offset);

    std::cout << "size: " << dataSize / sizeof(float) << std::endl;
    std::cout << "slice: " << header.dim[1] * header.dim[2] * header.dim[3] << std::endl;

    std::vector<float> data(dataSize / sizeof(float));
    file.read(reinterpret_cast<char*>(data.data()), dataSize);


    file.close();

    float max = *std::max_element(data.begin(), data.end());
    float min = *std::min_element(data.begin(), data.end());

    std::cout << max << " " << min << std::endl;

    size_t one_slice = header.dim[1] * header.dim[2] * header.dim[3];

    std::vector<std::vector<uint8_t>> res;

    for (size_t i = 0; i < dataSize / sizeof(float); i += one_slice) {
        std::vector<float> subc(one_slice);
        std::copy(data.begin() + i, data.begin() + i + one_slice, subc.begin());


        std::vector<uint8_t> sub(one_slice);
        for (size_t j = 0; j < one_slice; ++j) {
            sub[j] = uint8_t(((subc[j] - min) / (max - min)) * 255);
        }
        res.push_back(sub);
    }

    std::cout << "DONE\n";
    return { header, std::move(res)};
}

template <typename T>
NiftiFile4D loadNifti4dT(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    std::cout << "LOADING AS: " << typeid(T).name() << "\n";
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return { {}, {} };
    }

    nifti_1_header header;
    file.read(reinterpret_cast<char*>(&header), sizeof(nifti_1_header));


    std::cout << "dims: ";
    for (auto i : header.dim) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;
    std::cout << "intent: " << header.intent_name << " " << header.intent_code << std::endl;
    std::cout << "dtype: " << header.datatype << std::endl;
    std::cout << "bitpix: " << header.bitpix << std::endl;
    std::cout << "data scaling: " << header.scl_slope << " " << header.scl_inter << std::endl;
    std::cout << "range: " << header.cal_min << " " << header.cal_max << std::endl;
    std::cout << "vox: " << header.vox_offset << std::endl;
    std::cout << "dim_info: " << (int)header.dim_info << std::endl;

    size_t dataSize = sizeof(T);

    for (int i = 0; i < header.dim[0]; ++i)
        dataSize *= header.dim[i + 1];

    file.seekg(header.vox_offset);

    std::cout << "size: " << dataSize / sizeof(T) << std::endl;
    std::cout << "slice: " << header.dim[1] * header.dim[2] * header.dim[3] << std::endl;

    std::vector<T> data(dataSize / sizeof(T));
    file.read(reinterpret_cast<char*>(data.data()), dataSize);


    file.close();

    /*T max = *std::max_element(data.begin(), data.end());
    T min = *std::min_element(data.begin(), data.end());*/
    std::cout << "ds: " << data.size() << "\n";
    T max = data[0], min = data[0];
    for (const auto& i : data) {
        if (i < min) min = i;
        else if (i > max) max = i;
    }

    std::cout << max << " " << min << std::endl;

    size_t one_slice = header.dim[1] * header.dim[2] * header.dim[3];

    std::vector<std::vector<uint8_t>> res;

    for (size_t i = 0; i < dataSize / sizeof(T); i += one_slice) {
        std::vector<T> subc(one_slice);
        std::copy(data.begin() + i, data.begin() + i + one_slice, subc.begin());


        std::vector<uint8_t> sub(one_slice);
        for (size_t j = 0; j < one_slice; ++j) {
            sub[j] = uint8_t(((subc[j] - min) / (max - min)) * 255);
        }
        res.push_back(sub);
    }

    std::cout << "DONE\n";
    return { header, std::move(res) };
}

NiftiFile4D loadNifti4dt(const std::string& filepath) {

    auto header = getHeader(filepath.c_str());

    switch (header.datatype)
    {
    case 2: return loadNifti4dT<unsigned char>(filepath);
    case 4: return loadNifti4dT<signed short>(filepath);;
    case 8: return loadNifti4dT<signed int>(filepath);;
    case 16: return loadNifti4dT<float>(filepath);;
    case 64: return loadNifti4dT<double>(filepath);
    case 256: return loadNifti4dT<signed char>(filepath);
    case 512: return loadNifti4dT<unsigned short>(filepath);
    case 768: return loadNifti4dT<unsigned int>(filepath);;
    case 1024: return loadNifti4dT<long long>(filepath);
    case 1280: return loadNifti4dT<unsigned long long>(filepath);
    case 1536: return loadNifti4dT<long double>(filepath);;
    default: throw std::exception("Invalid Code");
    }
}