#include <iostream>
#include <memory>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include "DPServer.hpp"
#include "CustomSink.hpp"

int main(int argc, char const* argv[]) 
{

	// Initialize our logger
	std::unique_ptr<g3::LogWorker> logWorker{g3::LogWorker::createLogWorker()};
	logWorker->addSink(std::make_unique<CustomSink>(), &CustomSink::ReceiveLogMessage);
	g3::initializeLogging(logWorker.get());


	std::unique_ptr<DPServer> app = std::unique_ptr<DPServer>(new DPServer(argc, argv)); 
	app->start();

	return 0;
}
