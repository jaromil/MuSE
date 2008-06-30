#include <jutils.cpp>
#include <xmlrpc_ui.h>

XMLRPC_UI::XMLRPC_UI(int argc, char **argv, Stream_mixer *mix)
  : GUI(argc, argv, mix) {
  func("XMLRPC_UI::XMLRPC_UI()");
}

XMLRPC_UI::~XMLRPC_UI() { 
  func("XMLRPC_UI::~XMLRPC_UI()");
}
