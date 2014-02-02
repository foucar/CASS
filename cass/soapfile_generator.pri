# This function builds the soapfiles to be used. Ensure that the SOAP_BIN
# variable contains the the soapcpp2 binary with the appropriate flag to
# generate server or client code. The SOAP_INPUTFILE variable needs to contain
# the file h file to parse for code generation. The SOAP_OUTPUTFILE variable
# needs to contain the name of the output cpp file


SOAPMaker.name         = Generator for SOAP files
SOAPMaker.input        = SOAP_INPUTFILE
SOAPMaker.output       = $$SOAP_OUTPUTFILE
SOAPMaker.commands     = $$SOAP_BIN -i $$SOAP_INPUTFILE
SOAPMaker.variable_out = SOURCES
SOAPMaker.CONFIG      += target_predeps
SOAPMaker.clean       += soap?.*
SOAPMaker.clean       += *CASSsoap*
SOAPMaker.clean       += ns.xsd soapStub.h
QMAKE_EXTRA_COMPILERS += SOAPMaker

SOAPMaker2.name         = Adder for SOAP files to Sources
SOAPMaker2.input        = SOAP_INPUTFILE
SOAPMaker2.output       = soapC.cpp
SOAPMaker2.commands     = @true
SOAPMaker2.variable_out = SOURCES
SOAPMaker2.CONFIG      += target_predeps
QMAKE_EXTRA_COMPILERS  += SOAPMaker2
