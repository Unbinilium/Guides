#pragma once

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>

namespace tf {
#include "tensorflow/c/c_api.h"
}

namespace tpi {
    inline static void tf_version(void) {
        printf("[TPI] tensorflow version: %s\n", tf::TF_Version());
    }
    
    class model {
    public:
        inline void load(const char* model_path) {
            f = std::fopen(model_path, "rb");
            
            if (f == NULL) {
                std::perror("[TPI] load model failed");
                return;
            }
            
            std::fseek(f, 0, SEEK_END);
            f_size = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);
            
            data = std::malloc(f_size);
            std::fread(data, f_size, 1, f);
            std::fclose(f);
            
            buf->data             = data;
            buf->length           = f_size;
            buf->data_deallocator = [](void* data, [[maybe_unused]] size_t length) -> void {
                std::free(data);
            };
            
            tf::TF_GraphImportGraphDef(graph, buf, opts, status);
            tf::TF_DeleteImportGraphDefOptions(opts);
            
            if (tf::TF_GetCode(status) == tf::TF_OK) {
                std::printf("[TPI] load model success: %s\n", model_path);
            } else {
                std::printf("[TPI] load model failed %s\n", tf::TF_Message(status));
            }
            
            tf::TF_DeleteStatus(status);
        }
        
        inline void session(const char* input_oper_name, const int input_index, const char* output_oper_name, const int output_index) {
            input    = reinterpret_cast<tf::TF_Input*>(std::malloc(sizeof(tf::TF_Input)));
            input[0] = { tf::TF_GraphOperationByName(graph, input_oper_name), input_index };
            
            if (input[0].oper == NULL) {
                std::printf("[TPI] sessioning input failed: name %s at index %d\n", input_oper_name, input_index);
                return;
            }
            std::printf("[TPI] sessioning input success: name %s at index %d\n", input_oper_name, input_index);
            
            output    = reinterpret_cast<tf::TF_Output*>(std::malloc(sizeof(tf::TF_Output)));
            output[0] = { tf::TF_GraphOperationByName(graph, output_oper_name), output_index };
            
            if (output[0].oper == NULL) {
                std::printf("[TPI] sessioning output failed: name %s at index %d\n", output_oper_name, output_index);
                return;
            }
            std::printf("[TPI] sessioning output success: name %s at index %d\n", output_oper_name, output_index);
        }
        
    private:
        FILE* f      = NULL;
        long  f_size = 0;
        void* data   = NULL;
        
        tf::TF_Buffer*                buf    = tf::TF_NewBuffer();
        tf::TF_Graph*                 graph  = tf::TF_NewGraph();
        tf::TF_Status*                status = tf::TF_NewStatus();
        tf::TF_ImportGraphDefOptions* opts   = tf::TF_NewImportGraphDefOptions();
        
        tf::TF_Input*  input  = NULL;
        tf::TF_Output* output = NULL;
    };
}
