#pragma once

#include <App/Globals.h>

#include <pluginsdk/include/teamspeak/public_errors.h>
#include <pluginsdk/include/teamspeak/public_errors_rare.h>
#include <pluginsdk/include/teamspeak/public_definitions.h>
#include <pluginsdk/include/teamspeak/public_rare_definitions.h>
#include <pluginsdk/include/teamspeak/clientlib_publicdefinitions.h>
#include <pluginsdk/include/ts3_functions.h>

namespace TSPlugin {

	namespace Ts {
		using namespace TSPlugin::Global;

		static inline void CheckAndLogError(UINT error) {
			if (error != ERROR_ok) {
				CString strError = ErrorToString(error);
				Log::Warning(strError);
				assert(0 && strError);
			}
		}

		static inline CString ErrorToString(unsigned int error) {
			char* errormsg;
			if (ts3Functions.getErrorMessage(error, &errormsg) == ERROR_ok) {
				CString result(errormsg);
				ts3Functions.freeMemory(errormsg);
				return result;
			} else {
				return CString("Unknown error");
			}
		}

		static inline CStringA GetPreProcessorConfigValue(CStringA key) {
			char* ptr;
			CStringA result;
			unsigned int error = ts3Functions.getPreProcessorConfigValue(Global::connection, key, &ptr);
			CheckAndLogError(error);
			if (error == ERROR_ok) {
				result = ptr;
				ts3Functions.freeMemory(ptr);
			}
			return result;
		}

		static inline void SetPreProcessorConfigValue(CStringA key, CStringA value) {
			unsigned int error = ts3Functions.setPreProcessorConfigValue(Global::connection, key, value);
			CheckAndLogError(error);
		}

		static inline void SetClientSelfVariableAsInt(size_t key, int value) {
			UINT error = ts3Functions.setClientSelfVariableAsInt(Global::connection, key, value);
			CheckAndLogError(error);
		}

		static inline int GetClientSelfVariableAsInt(size_t key) {
			int value;
			UINT error = ts3Functions.getClientSelfVariableAsInt(Global::connection, key, &value);
			CheckAndLogError(error);
			return value;
		}

		static inline void FlushClientSelfUpdates() {
			UINT error = ts3Functions.flushClientSelfUpdates(Global::connection, nullptr);
			(void)error;
			//CheckAndLogError(error);
		}

		static const char* VoiceActivation = "vad";
		static const char* True = "true";
		static const char* False = "false";


	}

} // namespace TSPlugin 
 