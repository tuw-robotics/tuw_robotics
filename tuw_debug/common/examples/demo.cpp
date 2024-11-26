/**
 * @author Markus Bader <markus.bader@tuwien.ac.at>
 **/

#include <iostream>
#include <tuw/debug.hpp>

int main(int argc, char *argv[]){
        
    tuw::Debug debug;

    std::cout << "Demo tuw_debug says: " << debug.info() << std::endl;
    return 0;
}

