#pragma once

#ifndef _WINDOWS_ 
#include <Windows.h>
#endif

#include <vector>
#include <string>
#include <functional>
#include <memory>

#include <atlstr.h>


template<size_t T_SIZE>
CString get_app_path()
{
	static CString g_cstr_app;
	if ( g_cstr_app.IsEmpty() )
	{
		auto n = GetModuleFileName(NULL, g_cstr_app.GetBuffer(T_SIZE+1), T_SIZE );
		g_cstr_app.ReleaseBuffer(n);
		g_cstr_app = g_cstr_app.Left( g_cstr_app.ReverseFind(_T('\\'))+1 );
		
	}
	return g_cstr_app;
}

template<size_t T_SIZE>
CString get_last_err_message( DWORD error)
{
	CString cstr;
	DWORD bufLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		cstr.GetBuffer(T_SIZE),
		T_SIZE, NULL );
	cstr.ReleaseBuffer(bufLen);
	return cstr;
}

template<typename Func_Type>
bool load_shell( LPCTSTR filename, Func_Type func )
{
	HANDLE hf = CreateFile( filename, GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,NULL,NULL);
	std::vector<char> buff_temp;
	std::string line;
	DWORD dwLength,dwStep;
	
	if ( hf == INVALID_HANDLE_VALUE )
	{
		return false;
	}
	dwLength = GetFileSize(hf,&dwStep);
	buff_temp.resize( 4 * 1024, 0 );

	

	while ( 0 < dwLength )
	{
		if ( FALSE == ReadFile(hf, &buff_temp[0], (dwLength > 4 * 1024 ? 4 * 1024 : dwLength),&dwStep,NULL)
			|| dwStep == 0 )
		{
			break;
		}
		dwLength -= dwStep;

		auto itStart = buff_temp.begin(), itEnd = std::find(itStart,buff_temp.begin()+dwStep, '\n') ;
		
		while ( itStart != buff_temp.begin()+dwStep )
		{
			itEnd = std::find(itStart,buff_temp.begin()+dwStep, '\n');
			if ( itEnd == buff_temp.end() || itEnd == buff_temp.begin() + dwStep )
			{
				line.insert(line.end(),itStart,itEnd);
				break;
			}
			line.insert(line.end(),itStart, itEnd );
			line.erase(std::remove(line.begin(),line.end(),'\r'),line.end());
			func(line);
			line.clear();

			itStart = itEnd + 1; //jump the '\n'
		}
	}
	if ( line.empty() == false )
	{
		func(line); // '\n' may be not exist in last line
	}
	CloseHandle(hf);
	return true;
}

template<char TAG_F, char TAG_B, char KEY_B, char VALUE_E, typename T_FUNC>
bool load_ini( LPCTSTR filename, T_FUNC func )
{
	if ( KEY_B == '\0' || !func )
		return false;

	std::string tag;
	size_t npos;
	auto func_line = [&](std::string line){
		if ( line.empty() )
			return ;
		if ( line[0] == TAG_F )
		{
			char c[] = { TAG_B, VALUE_E, 0 };
			npos = line.find_first_of(c,1);
			if ( npos != std::string::npos )
			{
				tag = line.substr(1,npos);
				return ;
			}
		}
		
		std::string key,value;
		npos = line.find(KEY_B);
		if ( npos == std::string::npos )
		{
			key = line;
			func(tag,key,value);
			return ;
		}
		key = line.substr(0,npos);
		npos++;
		line.erase(0,npos);
		npos = line.find( VALUE_E );
		if ( npos == std::string::npos )
			value = line;
		else
			value = line.substr(0,npos);
		func(tag,key,value);
	};
	return load_shell(filename, func_line);
}

template<char KEY_B, char VALUE_E, typename T_FUNC>
bool load_ini<0,0>( LPCTSTR filename, T_FUNC func )
{
	auto func_line = [&](std::string line){
		std::string key,value;
		npos = line.find(KEY_B);
		if ( npos == std::string::npos )
		{
			key = line;
			func("",key,value);
			return ;
		}
		key = line.substr(0,npos);
		npos++;
		line.erase(0,npos);
		npos = line.find( VALUE_E );
		if ( npos == std::string::npos )
			value = line;
		else
			value = line.substr(0,npos);
		func("",key,value);
	};
	return load_shell(filename, func_line);
}

template<char TAG_B, char KEY_B, char VALUE_E, typename T_FUNC>
bool load_ini<0>( LPCTSTR filename, T_FUNC func )
{
	auto func_line = [&](std::string line){
		if ( line.empty() )
			return ;
		{
			npos = line.find(TAG_B);
			if ( npos != std::string::npos )
			{
				tag = line.substr(0,npos);
				return ;
			}
		}
		
		std::string key,value;
		npos = line.find(KEY_B);
		if ( npos == std::string::npos )
		{
			key = line;
			func(tag,key,value);
			return ;
		}
		key = line.substr(0,npos);
		npos++;
		line.erase(0,npos);
		npos = line.find( VALUE_E );
		if ( npos == std::string::npos )
			value = line;
		else
			value = line.substr(0,npos);
		func(tag,key,value);
	};
	return load_shell(filename, func_line);
}

template<char TAG_F, char KEY_B, char VALUE_E, typename T_FUNC>
bool load_ini<TAG_F, 0>( LPCTSTR filename, T_FUNC func )
{
	auto func_line = [&](std::string line){
		if ( line.empty() )
			return ;
		if ( line[0] == TAG_F )
		{
			npos = line.find(VALUE_E);
			if ( npos != std::string::npos )
			{
				tag = line.substr(1,npos);
			}
			else
				tag = line;
			return ;
		}
		
		std::string key,value;
		npos = line.find(KEY_B);
		if ( npos == std::string::npos )
		{
			key = line;
			func(tag,key,value);
			return ;
		}
		key = line.substr(0,npos);
		npos++;
		line.erase(0,npos);
		npos = line.find( VALUE_E );
		if ( npos == std::string::npos )
			value = line;
		else
			value = line.substr(0,npos);
		func(tag,key,value);
	};
	return load_shell(filename, func_line);
}

