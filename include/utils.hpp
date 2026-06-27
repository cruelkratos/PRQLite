#include "include/backend/memory_manager.hpp"
#include "include/table.hpp"      
#include <cstring>
#include <string>
#include <iomanip>
#include <iostream>


namespace db::utils{
	inline void printTuple(const db::memory::Tuple& tuple, 
                           const db::table::TableSchema& schema, 
                           std::ostream& os = std::cout) 
    {
        uint32_t current_offset = 0;
            
        for (const auto& col : schema.columns) {
            if (col.type == db::table::ColumnType::INT) {
                int val;
                std::memcpy(&val, &tuple.data[current_offset], sizeof(int));
                os << std::left << std::setw(15) << val;
                current_offset += sizeof(int);
            } 
            else if (col.type == db::table::ColumnType::BOOL) {
                bool val;
                std::memcpy(&val, &tuple.data[current_offset], sizeof(bool));
                os << std::left << std::setw(15) << (val ? "true" : "false");
                current_offset += sizeof(bool);
            } 
            else if (col.type == db::table::ColumnType::TEXT) {
                // Read up to the null terminator to avoid printing garbage bytes
                std::string val(&tuple.data[current_offset]); 
                os << std::left << std::setw(15) << val;
                current_offset += 255; 
            }
        }
    }
}