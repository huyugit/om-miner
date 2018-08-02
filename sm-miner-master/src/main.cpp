/*
 * Defines the entry point for the application.
 */

#include "except/Exception.h"
#include "except/ExitException.h"

#include "config/Config.h"
#include "config/CommandLineParser.h"

#include "ms-protocol/ms_packet.h"

#include "log/LogManager.h"
#include "log/BflLog.h"

#include "app/Application.h"

#include "pool/ShareValidator.h"

#include "version.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>



// Anonymous namespace.
namespace
{
    // Create Main logger.
    Logger& logger = LogManager::instance().createLogger();

}  // End of anonymous namespace.


// The application entry point.
int main(int argc, const char* argv[])
{
    ::printf("sm-miner %s %s (wm: 8), msp ver 0x%04x\n",
        swVersion, swVersionDate, MsPacket::VERSION);
    
    ::printf("Use \"--help\" switch to print command line options.\n\n");

    if (0)
    {
        ShareValidator::test();
        MsPacketUT::unitTest();
        return -1;
    }
    
    // Application return code.
    int retCode = EXIT_SUCCESS;
    bool exceptionCaught = false;

	/* initial log file */
	if(gMsErrorLog.initLogFile(MS_ERROR_LOG)){
		printf("Warning: %s open failed\n", MS_ERROR_LOG);
	}

	if(gStratumLog.initLogFile(NET_ERROR_LOG)){
		printf("Warning: %s open failed\n", NET_ERROR_LOG);
	}

    try
    {
        // Parse command line and update configuration accordingly.
        Application::parseCommandLine(argc, argv);
        
        // If started with "--help" switch, display command usage and quit.
        if (Application::config()->showUsage)
        {
            CommandLineParser::showUsage();
            throw ExitException();  // Exit the current try-catch block.
        }
        
        Application::configPostProcessing();

        // Initialize the application.
        Application::init();
        
        // Run the application.
        Application::run();
    }
    catch (const ExitException& )
    {
        exceptionCaught = true;
        ;  // Do nothing.
    }
    catch (const Exception& e)
    {
        exceptionCaught = true;
        LOG_ERROR(logger) << e.what() << "\n" << Logger::flush();
        retCode = EXIT_FAILURE;
    }
    catch (...)
    {
        exceptionCaught = true;
        LOG_ERROR(logger) << "Unknown error.\n" << Logger::flush();
        retCode = EXIT_FAILURE;
    }
    
    if (!exceptionCaught)
    {
        // Uninitialize the application.
        Application::done();
    }
    
    return retCode;
}
