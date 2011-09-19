#!/usr/bin/python

import sys

LES_LOGGERCHANNEL_FLAGS_FATAL = 					(1<<0)
LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT = 	(1<<1)
LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT = 		(1<<2)
LES_LOGGERCHANNEL_FLAGS_DEFAULT = LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT | LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT

FLAGS_DEFAULT = LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT | LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT

CHANNEL_FATAL_ERROR = 0
CHANNEL_ERROR = 			1
CHANNEL_WARNING = 		2
CHANNEL_LOG = 				3
LOGGER_NUM_DEFAULT_CHANNELS = 4

__DEFAULT_LOG_OUTPUT_FILE__ = "log_py.txt"

__s_errorFlag__ = False
__s_channels__ = []
__s_defaultChannels__ = []

def Init():
	# Create default channels
	defaultFlags = FLAGS_DEFAULT

	for i in range(LOGGER_NUM_DEFAULT_CHANNELS):
		__s_defaultChannels__.append(None)

	__s_defaultChannels__[CHANNEL_FATAL_ERROR] = CreateChannel("FATAL_ERROR", "FATAL_ERROR: ", __DEFAULT_LOG_OUTPUT_FILE__, 
										 																		 defaultFlags | LES_LOGGERCHANNEL_FLAGS_FATAL)

	__s_defaultChannels__[CHANNEL_ERROR] = CreateChannel("ERROR", "ERROR: ", __DEFAULT_LOG_OUTPUT_FILE__, defaultFlags)
	__s_defaultChannels__[CHANNEL_WARNING] = CreateChannel("WARNING", "WARNING: ", __DEFAULT_LOG_OUTPUT_FILE__, defaultFlags)
	__s_defaultChannels__[CHANNEL_LOG] = CreateChannel("LOG", "", __DEFAULT_LOG_OUTPUT_FILE__, defaultFlags)

def Shutdown():
	# Loop over the channels closing them down
	del __s_defaultChannels__
	del __s_channels__
	

def GetErrorStatus():
	return __s_errorFlag__

def ClearErrorStatus():
	__s_errorFlag__ = False

def CreateChannel(nickName, prefix, outputFileName, flags):
	newChannel = LES_LoggerChannel(nickName, prefix, outputFileName, flags)
	__s_channels__.append(newChannel)
	return newChannel

def GetDefaultChannel(channel):
	if (channel < 0) or (channel >= LOGGER_NUM_DEFAULT_CHANNELS):
		return None

	return __s_defaultChannels__[channel]

def SetChannelOutputFileName(channel, fname):
	channelObj = GetDefaultChannel(channel)
	if channelObj != None:
		channelObj.__SetOutputFileName__(fname)

def SetChannelFlags(channel, flags):
	channelObj = GetDefaultChannel(channel)
	if channelObj != None:
		channelObj.SetFlags(flags)

def GetChannelFlags(channel):
	channelObj = GetDefaultChannel(channel)
	if channelObj != None:
		return channelObj.GetFlags()

	return 0

def SetFatal(channel, fatal):
	channelObj = GetDefaultChannel(channel)
	if channelObj != None:
		channelObj.ChangeFlags(LES_LOGGERCHANNEL_FLAGS_FATAL, fatal)

def SetConsoleOutput(channel, consoleOutput):
	channelObj = GetDefaultChannel(channel)
	if channelObj != None:
		channelObj.ChangeFlags(LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT, consoleOutput)

def SetFileOutput(channel, fileOutput):
	channelObj = GetDefaultChannel(channel)
	if channelObj != None:
		channelObj.ChangeFlags(LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT, fileOutput)

def FatalError(fmt, *args):
	__SetErrorStatus__()
	channelObj = GetDefaultChannel(CHANNEL_FATAL_ERROR)
	if channelObj != None:
		channelObj.__InternalOutput__(fmt, args)

def Error(fmt, *args):
	__SetErrorStatus__()
	channelObj = GetDefaultChannel(CHANNEL_ERROR)
	if channelObj != None:
		channelObj.__InternalOutput__(fmt, args)

def Warning(fmt, *args):
	channelObj = GetDefaultChannel(CHANNEL_WARNING)
	if channelObj != None:
		channelObj.__InternalOutput__(fmt, args)

def Log(fmt, *args):
	channelObj = GetDefaultChannel(CHANNEL_LOG)
	if channelObj != None:
		channelObj.__InternalOutput__(fmt, args)

def __SetErrorStatus__():
	__s_errorFlag__ = True

class LES_LoggerChannel():
	def __init__(self, nickName = "", prefix = "", outputFileName = "", flags = 0):
		self.__m_nickName__ = nickName
		self.__m_prefixStr__ = prefix
		self.__m_flags__ = flags
		self.__m_outputFileName__ = outputFileName
		if outputFileName != "":
			fh = open(self.__m_outputFileName__, "w")
		if fh != None:
			fh.close()

	def Error(self, fmt, *args):
		__SetErrorStatus__()
		self.__InternalOutput__(fmt, args)

	def Print(self, fmt, *args):
		self.__InternalOutput__(fmt, args)

	def SetFlags(self, flags):
		self.__m_flags__ = flags

	def ChangeFlags(self, flags, enable):
		currentFlags = self.__m_flags__
		newFlags = currentFlags & ~flags
		if enable:
	 		newFlags = newFlags | flags
		__m_flags__ = newFlags

	def GetFlags(self):
		return self.__m_flags__

	def __SetOutputFileName__(self, fname):
		if fname != self.__m_outputFileName__:
			self.__m_outputFileName__ = fname
			fh = open(self.__m_outputFileName__, "w")
			if fh == None:
				return
			fh.close()

	def __InternalOutput__(self, fmt, argsList):
		flags = self.__m_flags__
		prefix = self.__m_prefixStr__
		outputBuffer = prefix + fmt % argsList

		if flags & LES_LOGGERCHANNEL_FLAGS_CONSOLE_OUTPUT:
			print outputBuffer

		if flags & LES_LOGGERCHANNEL_FLAGS_FILE_OUTPUT:
			fileName = self.__m_outputFileName__
			fh = open(fileName, "a")
			if fh != None:
				fh.write(outputBuffer)
				fh.write("\n")
				fh.flush()
				fh.close()

		if flags & LES_LOGGERCHANNEL_FLAGS_FATAL:
			__SetErrorStatus__()
			sys.exit(-1)

def runTest():
	Init()
	verbose = True
	SetConsoleOutput(CHANNEL_LOG, verbose)
	SetChannelOutputFileName(CHANNEL_WARNING, "warning.txt")
	SetChannelOutputFileName(CHANNEL_ERROR, "error.txt")
	SetChannelOutputFileName(CHANNEL_LOG, "console.txt")

	SetFileOutput(CHANNEL_LOG, True)
	SetConsoleOutput(CHANNEL_LOG, True)

	jakeChannel = CreateChannel("Jake", "Custom: ", "jake.txt", LES_LOGGERCHANNEL_FLAGS_DEFAULT)
	jakeChannel.Print("Jake")
	jakeChannel.Print("Jake %d %f %s", 10, 10, "Hello")

	ClearErrorStatus()
	Log("0 ErrorStatus:%d", GetErrorStatus())
	argv = []
	for i in range(10):
		argv.append("Jake " + str(i))
		jakeChannel.Print("arg[%d] '%s'", i, argv[i])
		Log("arg[%d] '%s'", i, argv[i])

	Log("0 ErrorStatus:%d", GetErrorStatus())
	SetFatal(CHANNEL_WARNING, False)
	SetFileOutput(CHANNEL_WARNING, True)
	Warning("A test warning")
	Log("0 ErrorStatus:%d", GetErrorStatus())

	SetFatal(CHANNEL_ERROR, False)
	SetFileOutput(CHANNEL_ERROR, True)
	Error("A test error")
	Log("1 ErrorStatus:%d", GetErrorStatus())
	ClearErrorStatus()
	Log("0 ErrorStatus:%d", GetErrorStatus())

	SetFatal(CHANNEL_FATAL_ERROR, False)
	SetFileOutput(CHANNEL_FATAL_ERROR, False)
	Log("0 ErrorStatus:%d", GetErrorStatus())
	FatalError("A test fatal error")
	Log("1 ErrorStatus:%d", GetErrorStatus())

	SetFatal(CHANNEL_FATAL_ERROR, True)

if __name__ == '__main__':
	runTest()
