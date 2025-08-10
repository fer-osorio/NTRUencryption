#include<stddef.h>
#include<functional>

namespace DataGeneration{

std::vector<std::byte> dataSource(std::function<std::byte (size_t)> generator, size_t size);
std::vector<std::byte> simpleDataSource(size_t);

}