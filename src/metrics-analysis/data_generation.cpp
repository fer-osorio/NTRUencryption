#include"../../include/metrics-analysis/data_generation.hpp"

std::vector<std::byte> DataGeneration::dataSource(std::function<std::byte (size_t)> generator, size_t size){
    std::vector<std::byte> data(size);
    for(size_t i = 0; i < size; i++) data[i] = generator(i);
    return data;
}

std::vector<std::byte> DataGeneration::simpleDataSource(size_t size){
    return dataSource(
        [](size_t i) ->std::byte {
            size_t FF = 0xFF, q = (i & ~FF)>>8, r = i & FF;                     // i = 256·q + r
            return std::byte((q+r) & FF);                                       // (q+i) % 256
        }
        ,size);
}
