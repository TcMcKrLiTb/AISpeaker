#include <gui/ethernettester_screen/EthernetTesterView.hpp>

#ifndef SIMULATOR
#include <cstdio>

#include "network.h"
#endif

EthernetTesterView::EthernetTesterView()
{

}

void EthernetTesterView::setupScreen()
{
    EthernetTesterViewBase::setupScreen();
}

void EthernetTesterView::tearDownScreen()
{
    EthernetTesterViewBase::tearDownScreen();
}

void EthernetTesterView::function1()
{
#ifndef SIMULATOR
    startPostRequest();
    printf("lwip_client_example called\r\n");
#endif
}
