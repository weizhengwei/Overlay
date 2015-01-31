#pragma once

#include <string>
#include <vector>

class ICSVRow
{
public:
	virtual ~ICSVRow(){}

public:
	virtual int					get_index() = 0;
	virtual const std::wstring&	get_colum_string(int colum) = 0;
	virtual void				set_colum_string(int colum,const std::wstring& str) = 0;
	virtual int					get_size() = 0;
	virtual void				set_size(int size) = 0;
};

class ICSVTool
{
public:
	virtual ~ICSVTool(void){}
	virtual void release() = 0;
	virtual void clear() =0;  

	virtual bool 	load(const std::wstring& path,bool checkBom = true) = 0;
	virtual bool	save(const std::wstring& path,bool saveBom = true) = 0;
	
	virtual ICSVRow*	create_raw(int index = -1, int colum_count = 0) = 0;
	virtual void		release_raw(ICSVRow* row) = 0;
	
	virtual int			get_row_count() = 0;
	virtual ICSVRow*	get_row_by_index(int index) = 0;
	
	virtual bool		insert_row(ICSVRow* row, int index = -1) = 0;
	virtual bool		remove_row(ICSVRow* row) = 0;
	
	virtual const std::wstring&  get_csv_string(int row, int colum) = 0;
	virtual void				 set_csv_string(int row, int colum, const std::wstring& str) = 0;
};

extern ICSVTool* create_csv_tool();
extern void release_csv_tool(ICSVTool* pTool);