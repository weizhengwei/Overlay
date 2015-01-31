#include "StdAfx.h"
#include "data/CSVTool.h"
#include <fstream>
#include <assert.h>
#include <algorithm>

#define FALSE (0)

#ifdef _DEBUG
#define ASSERT(exp) assert(exp)
#else
#define ASSERT(exp) 
#endif

//////////////////////////////////////////////////////////////////////////
using namespace std;

static const wstring DEFALUT_NULL_STR = _T("");

void replace_all_char(std::wstring& str, wchar_t wch, wchar_t rep)
{
	size_t len = str.length();
	for (size_t i=0; i<len; ++i)
	{
		if (str[i] == wch)
		{
			str[i] = rep;
		}
	}
}

static std::wstring pack_item_string(const std::wstring& str)
{
	if (str.empty())
		return str;

	if (str.find('"') != std::wstring::npos || str.find(',') != std::wstring::npos)
	{
		std::wstring result;
		result.push_back('"');
		for (int i=0; i<(int)str.length(); i++)
		{
			if (str.at(i) == '"')
			{
				result.push_back('"');
				result.push_back('"');
			}
			else
			{
				result.push_back(str.at(i));
			}
		}
		result.push_back('"');

		replace_all_char(result, L'\t', L' ');
		return result;
	}
	else 
	{	
		replace_all_char(const_cast<std::wstring&>(str), L'\t', L' ');
		return str;
	}


};
//////////////////////////////////////////////////////////////////////////
CSVRow::CSVRow() 
{
	m_colum_vec.clear();
	m_index = -1;
}

CSVRow::~CSVRow()
{	
}

int	CSVRow::get_index()
{
	return m_index;
}

const std::wstring& CSVRow::get_colum_string(int colum)
{
	
	if (colum >= 0 && colum < (int)m_colum_vec.size())
	{
		return m_colum_vec[colum];
	}
	else
	{
		ASSERT(FALSE && _T("get_colum_string : colum 越界"));
		return DEFALUT_NULL_STR;
	}
}

void CSVRow::set_colum_string(int colum,const std::wstring& str)
{
	if (colum >= 0 && colum < (int)m_colum_vec.size())
	{
		m_colum_vec[colum] = str;
	}
	else
	{
		ASSERT(FALSE && _T("get_colum_string : colum 越界"));
	}
}

int	CSVRow::get_size()
{
	return (int)m_colum_vec.size();
}

void CSVRow::set_size(int size)
{
	return m_colum_vec.resize(size);
}


//////////////////////////////////////////////////////////////////////////
CSVTool::CSVTool(void)
{
	m_row_vec.clear();
}

CSVTool::~CSVTool(void)
{
	clear();
}

void CSVTool::release()
{
	delete this;
}

void CSVTool::clear()
{
	RowSet::iterator  it_end = m_free_set.end();
	RowSet::iterator it_cur = m_free_set.begin();
	while (it_cur != it_end)
	{
		delete *it_cur;
		++it_cur;
	}

	m_free_set.clear();
	m_row_vec.clear();
}

bool CSVTool::load(const std::wstring& path, bool checkBom)
{
	FILE* pFile = _wfopen(path.c_str(), _T("rb"));
	if (pFile == 0)
	{
		return false;
	}
	clear();

	bool bRet = false;
	do 
	{
		if (checkBom)
		{
			unsigned short head;
			if(fread(&head, sizeof(unsigned short), 1, pFile) < 1)
			{
				break;
			}

			if (head != 0xFEFF)
			{
				break;
			}
		}
	
		if (!parse(pFile))
		{	
			break;
		}

		bRet = true;
	} while (0);
	
	fclose(pFile);
	
	return bRet;
}

bool CSVTool::save(const std::wstring& path, bool saveBom)
{
	FILE* pFile = _wfopen(path.c_str(), _T("wb"));
	if (pFile == 0)
	{
		return false;
	}

	if (saveBom)
	{
		unsigned short head = 0xFEFF;
		if ((fwrite(&head, sizeof(head),1, pFile)) < 1)
		{
			fclose(pFile);
			return false;
		}
	}

	bool is_ok = true;
	int row_count = (int)m_row_vec.size();
	for (int i=0;i<row_count; ++i)
	{
		ICSVRow* row = get_row_by_index(i);
		int colum_count = row->get_size();
		std::wstring line;
		for (int j=0; j<colum_count-1; j++)
		{
			line.append(pack_item_string(row->get_colum_string(j)));
			line.append(1, L'\t');
		}

		if (colum_count > 0)
		{
			line.append(pack_item_string(row->get_colum_string(colum_count - 1)));
		}

		line += _T("\r\n");
		
		if(fwrite(line.c_str(), line.size() * sizeof(wchar_t),1, pFile) < 1)
		{
			is_ok = false;
			break;
		}
	}

	fclose(pFile);
	return is_ok;
}

ICSVRow* CSVTool::create_raw(int index, int colum_count)
{
	CSVRow* row = new CSVRow;
	
	colum_count = colum_count < 0 ? 0 : colum_count;
	row->set_size(colum_count);

	if (index < 0 || index >= (int)m_row_vec.size())
	{
		index = (int)m_row_vec.size() - 1;
		index = index < 0 ? 0 : index;
		m_row_vec.push_back(row);
	}
	else
	{
		insert_row(row, index);
	}

	m_free_set.insert(row);
	
	return row;
}

void CSVTool::release_raw(ICSVRow* row)
{
	if (row != NULL)
	{
		remove_row(row);
	
		if (m_free_set.find((CSVRow*)row) != m_free_set.end())
		{
			m_free_set.erase((CSVRow*)row);
			delete row;
		}
		else
		{
			ASSERT(FALSE && _T("删除不存在的row"));
		}
	}
}

int	CSVTool::get_row_count()
{
	return (int)m_row_vec.size();
}

void CSVTool::update_order()
{
	int count = (int)m_row_vec.size();
	for (int i=0; i<count; i++)
	{
		m_row_vec[i]->set_index(i);
	}
}

ICSVRow* CSVTool::get_row_by_index(int index)
{
	if (index >=0 && index < (int)m_row_vec.size())
	{
		return m_row_vec[index];
	}

	return NULL;
}

bool CSVTool::insert_row(ICSVRow* row, int index)
{
	if (row == NULL)
	{
		ASSERT(row != NULL);
		return false;
	}
	
	if (index < (int)m_row_vec.size() && index >= 0)
	{
		RowVec::iterator it = m_row_vec.begin() + index;
		m_row_vec.insert(it,(CSVRow*)row);

		update_order();
		return true;
	}
	else
	{
		return false;
	}
}

bool CSVTool::remove_row(ICSVRow* row)
{
	RowVec::iterator it = std::find(m_row_vec.begin(), m_row_vec.end(), row);
	if (it != m_row_vec.end())
	{
		m_row_vec.erase(it);
		update_order();

		return true;
	}
	else
		return false;
}

const std::wstring&  CSVTool::get_csv_string(int row, int colum)
{
	if (row < (int)m_row_vec.size() && row >= 0)
	{
		ICSVRow* row_data = get_row_by_index(row);
		if (colum >= 0 && colum < row_data->get_size())
		{
			return row_data->get_colum_string(colum);
		}
	}

	return DEFALUT_NULL_STR;
}

void CSVTool::set_csv_string(int row, int colum, const std::wstring& str)
{
	if (row < (int)m_row_vec.size() && row >= 0)
	{
		ICSVRow* row_data = get_row_by_index(row);
		if (colum >= 0 && colum < row_data->get_size())
		{
			row_data->set_colum_string(colum,str);
		}
	}
}

bool getline(FILE* pFile, wstring& outline)
{
	if (feof(pFile) != 0 || ferror(pFile))
	{
		return false;
	}

	bool ret = true;
	wchar_t wch;
	wch = fgetwc(pFile);
	while(wch != WEOF)
	{
		if (wch == L'\r')
		{
			wchar_t next_char = fgetwc(pFile);
			if (next_char == '\n')
			{
				break;
			}
			else
			{
				fputwc(wch, pFile);
				break;
			}
		}
		else if (wch == L'\n')
		{
			break;
		}
		else
		{
			outline.append(1, wch);
			wch = fgetwc(pFile);
		}
	}

	return ret;
}

bool CSVTool::parse(FILE* pFile)
{
 	int line_num = 0;
 	wstring message;
 	while(getline(pFile,message))
 	{
 		if(!parse_line(message, ++line_num))
 			return false;

		message.clear();
 	}

	return true;
}

bool CSVTool::parse_line(const std::wstring& line, int line_number)
{
	if (line.empty())
		return true;

	CSVRow* row = (CSVRow*)create_raw(-1,0);
	bool is_parse_error = false;

	enum CheckState{state_first_check, state_quot_first, state_normal};

	CheckState state = state_first_check;
	size_t line_len = line.length();
	wstring item_str;
	for (size_t i=0; i<line_len && !is_parse_error; ++i)
	{
		wchar_t c = line.at(i);
		switch (state)
		{
		case state_first_check:
			if (c == L'"')
			{
				state = state_quot_first;
			}
			else if(c == L'\t')
			{
				row->push_colum(item_str);
				item_str.clear();
			}
			else
			{
				item_str.push_back(c);
				state = state_normal;
			}
			break;
		case state_quot_first:
			if (c == L'"')
			{
				if (i >= line_len-1)
				{
					state = state_first_check;
					row->push_colum(item_str);
					item_str.clear();
					break;
				}

				wchar_t next_char = line.at(i+1);
				if (next_char == L'"')
				{
					i += 1;
					item_str.push_back(c);
				}
				else
				{
					if (next_char == L'\t')
					{
						i += 1;
						state = state_first_check;
						row->push_colum(item_str);
						item_str.clear();
					}
					else
					{
						//解析引号表达式出错
						is_parse_error = true;
					}
				}
			}
			else
			{
				item_str.push_back(c);
			}
			break;
		case state_normal:
			if (c == L'\t')
			{
				state = state_first_check;
				row->push_colum(item_str);
				item_str.clear();
			}
			else
			{
				item_str.push_back(c);
			}
			break;
		default:
			ASSERT(FALSE && "parseline error");
		}
	}

	if (!is_parse_error)
	{
		wchar_t last_char = line.at(line_len-1);
		if (state == state_first_check)
		{
			if (last_char == L'\t')
			{
				row->push_colum(_T(""));
			}
		}
		else 
		{
			row->push_colum(item_str);
		}
		
	}
	return !is_parse_error;
}


ICSVTool* create_csv_tool()
{
	return new CSVTool;
}

void release_csv_tool(ICSVTool* pTool)
{
	delete pTool;
}