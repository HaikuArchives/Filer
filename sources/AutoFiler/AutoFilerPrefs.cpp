#include <Application.h>
#include "PrefWindow.h"

class App : public BApplication
{
public:
	App(void);
};


App::App(void)
 :	BApplication("application/x-vnd.dw-AutoFilerPrefs")
{
	PrefWindow *prefwin = new PrefWindow();
	prefwin->Show();
}


int
main(void)
{
	App().Run();
	return 0;
}