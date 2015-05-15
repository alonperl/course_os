//
// Created by griffonn on 15/05/2015.
//
#include <iostream>
#include "Te.h"

int Te::a = 4;

int Te::getA()
{
    return a;
}

int main()
{
    Te t;
    std::cout << Te::getA() << std::endl;
    return 0;
}