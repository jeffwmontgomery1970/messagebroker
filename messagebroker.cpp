#include <iostream>
#include <vector>
#include <string>
#include "messagebroker.h"
#include "gre/sbio_wrapper.h"

using namespace std;

Broker::Broker()
{
   int result = sbio_create_receive_channel(BROKER_NAME, GRE_IO_TYPE_RDONLY | GRE_IO_TYPE_WRONLY, &handle);
   if (result != 0) 
   {
      cerr << "Failed to create " << BROKER_NAME << " channel" << endl;
      exit(1);
   }
};

Broker::~Broker()
{
   sbio_destroy_channel(handle);
}

int main()
{
   vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};

   for (const string& word : msg)
   {
      cout << word << " ";
   }
   cout << endl;
};