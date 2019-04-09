#pragma once
#include <exception>
#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>
#include "..\..\include\pplx\task_canceled.h"
#include "..\..\include\cpprest\http_exception.h"

#define CPPREST_BEGIN_NATIVE_TRY try {

#define CPPREST_END_NATIVE_TRY } \
		catch (pplx::task_canceled &ex) { \
			throw gcnew System::Threading::Tasks::TaskCanceledException(msclr::interop::marshal_as<System::String^>(ex.what())); \
		} catch (std::exception &ex) { \
			throw gcnew System::Exception(msclr::interop::marshal_as<System::String^>(ex.what())); \
		} catch (System::Exception ^) { \
			throw; \
		} catch (...) {  throw /*gcnew System::Exception("unexpected exception from c++")*/; } 

#define CPPREST_BEGIN_MANAGED_TRY try	{
#define CPPREST_END_MANAGED_TRY } \
			catch (std::exception &) { \
				throw; \
			} catch (System::Threading::ThreadAbortException^ ex) {\
				throw pplx::task_canceled(msclr::interop::marshal_as<std::string>(ex->ToString()).c_str()); \
			} catch (System::OperationCanceledException^ ex) {\
				throw pplx::task_canceled(msclr::interop::marshal_as<std::string>(ex->ToString()).c_str());\
			} catch (System::AggregateException ^aex) { \
				std::string msg; \
				for each (System::Exception^ ex in aex->InnerExceptions) \
					msg += msclr::interop::marshal_as<std::string>(ex->ToString()) + ". "; \
				throw std::runtime_error(msg); \
			} catch (System::Net::WebException ^ex) { \
				if(ex->Response != nullptr && ex->Response->GetType() == System::Net::HttpWebResponse::typeid) \
				{\
					auto response = (System::Net::HttpWebResponse^)(ex->Response);\
					throw http_exception(msclr::interop::marshal_as<std::string>(ex->ToString() + " with .Net StatusDescription: '"+(response->StatusDescription)+"' with .Net StatusCode: '"+response->StatusCode.ToString()+"' ("+((int)(response->StatusCode)).ToString()+")")); \
				}\
				else \
					throw http_exception(msclr::interop::marshal_as<std::string>(ex->ToString())); \
			} catch (System::Exception ^ex) { \
				throw std::runtime_error(msclr::interop::marshal_as<std::string>(ex->ToString())); \
			} catch (...) { throw; }


