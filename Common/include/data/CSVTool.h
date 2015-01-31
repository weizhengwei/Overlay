#pragma once
#include "CSVToolInterface.h"
#include <fstream>
#include <set>

class CSVRow : public ICSVRow
{
public:
	CSVRow();
	~CSVRow();

public:
	virtual int					get_index();
	virtual const std::wstring&	get_colum_string(int colum);
	virtual void				set_colum_string(int colum, const std::wstring& str);
	virtual int					get_size();
	virtual	void				set_size(int size);

	void	set_index(int index){m_index = index;}
	void	push_colum(const std::wstring& str)
	{
		m_colum_vec.push_back(str);
	}
private:
	std::vector<std::wstring> m_colum_vec;
	int	m_index;
};


class CSVTool : public ICSVTool
{
public:
	CSVTool(void);
public:
	~CSVTool(void);

public://ICSVTool interface
	virtual void release();

	virtual bool 	load(const std::wstring& path, bool checkBom);
	virtual bool	save(const std::wstring& path, bool saveBom);

	virtual ICSVRow*	create_raw(int index,int colum_count);
	virtual void		release_raw(ICSVRow* row);

	virtual int			get_row_count();
	virtual ICSVRow*	get_row_by_index(int index);

	virtual bool		insert_row(ICSVRow* row, int index);
	virtual bool		remove_row(ICSVRow* row);

	virtual const std::wstring&  get_csv_string(int row, int colum);
	virtual void				 set_csv_string(int row, int colum, const std::wstring& str);

	virtual void clear();
private:
	bool parse(FILE* pFile);
	bool parse_line(const std::wstring& line, int line_number);
	void update_order();
private:
	typedef std::vector<CSVRow*> RowVec;
	RowVec m_row_vec;

	typedef std::set<CSVRow*> RowSet;
	RowSet m_free_set;
};
