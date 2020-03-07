all:
	g++ -o frdds main.cpp dds.cpp mainframe.cpp `wx-config --cxxflags` `wx-config --libs`
