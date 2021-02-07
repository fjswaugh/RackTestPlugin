#include "plugin.hpp"

rack::Plugin* gPlugin;

void init(rack::Plugin* p)
{
    gPlugin = p;

    gPlugin->addModel(gTestModel);
}
