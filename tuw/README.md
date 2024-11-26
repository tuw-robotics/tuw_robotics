# tuw_ros2
Utils for ros2 such as an enhanced base class node. 
The content is header only.
# tuw::Node
## usage
```cpp
#ifndef MY_PKG__MYNODE_HPP_
#define MY_PKG__MYNODE_HPP_

#include <memory>
#include <tuw_ros2_utils/node.hpp>

class MyNode : public tuw::Node
{
    int loop_rate_;
    double threshold_;
    std::string frame_id_;
public:
    MyNode(const std::string &node_name): Node(node_name){
        declare_parameters();
        read_static_parameters();
        read_dynamic_parameters();
    }

    void declare_parameters(){
        declare_parameters_with_description("loop_rate", 5, "loop rate in seconds");
        declare_parameters_with_description("threshold", 1.5, "threshold [m]", 0.0, 10.0, 0.01);
        declare_parameters_with_description("frame_id", "map", "map frame");
    }
    void read_static_parameters(){
        get_parameter_and_log("loop_rate", loop_rate_);
    }                  
    bool read_dynamic_parameters(){
        static bool first_call = true;                     /// to identify the first call
        bool changes = false;                              /// used to identify changes
        update_parameter_and_log("threshold", threshold_); /// updates threshold_ 
        update_parameter_and_log("frame_id", frame_id_);   /// updates frame_id_ 
        first_call = false;                                /// after the first all the varibale stays on false
        return changes;
    }
}
#endif // TUW_GRAPH_GENERATOR__GRAPH_SPLIT_HPP_
```
