#include "libs.h"
#include "sg_pi_app.h"
#include "global_var.h"

SGPiApp* pi = NULL;
/*
    Arguments description:
        -f: specify filename of the stored save data filename
        -v: velocity setpoint
*/
int main(int argc, char** argv)
{
	g_output_filename = new char[strlen("out/last_session.dat") + 1];
	strcpy(g_output_filename, "out/last_session.dat");

	// Process the arguments
	int n = argc;
	for (int i = 1; i<n; i += 2)
	{
		if (i == n - 1) {
			ERROR_PRINT("Error");
		}
		if (strcmp(argv[i], "-f") == 0) { // Output filename
			int len = strlen(argv[i + 1]);
			g_output_filename = new char[len + 1];
			strcpy(g_output_filename, argv[i + 1]);
		}
		else if (strcmp(argv[i], "-v") == 0) { // Velocity command
			sscanf(argv[i + 1], "%f", &g_initial_velocity_setpoint);
		}
		else {
			// Nothing
		}
	}
  pi = new SGPiApp();

  if (!pi->Init())
  {
    DEBUG_PRINT("Initialization failed!");
    delete[] g_output_filename;
    return EXIT_FAILURE;
  }

  pi->Run();

  delete pi;
  // delete[] g_output_filename; // delete or add???

  return EXIT_SUCCESS;
}
