#include <QFile>
#include <QTextStream>

#include "csvParser.h"

csvParser::csvParser() :
    m_ignore_first_n_lines(0),
    m_max_lines_to_parse(-1),
    m_num_of_columns_expected(-1),
    m_enable_empty_cells(false),
    m_separator(";"),
    m_first_line_is_header(true),
    m_simplifie_cells(true),
    m_stop_on_first_error(true)
{
}

csvParser::~csvParser()
{
}

void csvParser::setNumberOfFirstLinesToIgnore(long num)
{
    m_ignore_first_n_lines = num;
}

void csvParser::setMaxLinesToParse(long max)
{
    m_max_lines_to_parse = max;
}

void csvParser::setNumberOfExpectedColums(long num)
{
    m_num_of_columns_expected = num;
}

void csvParser::enableEmptyCells(bool state)
{
    m_enable_empty_cells = state;
}

void csvParser::setSeparator(QString sep)
{
    m_separator = sep;
}

void csvParser::firstLineIsHeader(bool state)
{
    m_first_line_is_header = state;
}

void csvParser::simplifieCells(bool state)
{
    m_simplifie_cells = state;
}

void csvParser::stopOnFirstErrorDetected(bool state)
{
    m_stop_on_first_error = state;
}

bool csvParser::parse(const QString& pathfilename, csvData& out_data, QStringList *out_error_msg, QStringList *out_warn_msg)
{
    bool ok=true;

    // Try to read file
    QFile file(pathfilename);
    if ( !file.open(QFile::ReadOnly | QFile::Text) )
    {
        if (out_error_msg) {
            out_error_msg->append(QObject::tr("Cannot read file %1").arg(file.fileName()));
        }
        return false;
    }

    QTextStream in(&file);

    // ignore n first lines
    for (int i=0; i<m_ignore_first_n_lines; i++) {
        in.readLine();
    }

    int line_num = 0;
    QString line;
    //int found_columns_on_first_line;
    while ((line = in.readLine()) != NULL) {
      line_num++;
      QStringList split_line = line.split(m_separator);
      int found_columns = split_line.size();
      int num_of_columns_expected;
      // The number of columns found on first line becomes the reference to compare with each lines
      if (line_num == 1) {
         if (m_num_of_columns_expected == -1) num_of_columns_expected = found_columns; // auto-detect
         else num_of_columns_expected = m_num_of_columns_expected;
      }

      if (found_columns != num_of_columns_expected) {
          if (out_error_msg) {
              out_error_msg->append(QObject::tr("Line %1 : expected %2 columns / observed %3").arg(line_num).arg(num_of_columns_expected).arg(found_columns));
          }
          if (m_stop_on_first_error) return false;
          ok = false;
          continue;
      }

      for (int i=0; i< found_columns; i++) {
          // simplifie cells
          if (m_simplifie_cells) {
              split_line[i] = split_line[i].simplified();
          }
          // check for empty cells
          if (split_line.at(i) == "") {
              QString msg = QObject::tr("Line %1 / column %2 : empty field").arg(line_num).arg(i+1);
              if (m_enable_empty_cells) {
                  if (out_warn_msg) out_warn_msg->append(msg);
              }
              else {
                  if (out_error_msg) out_error_msg->append(msg);
                  if (m_stop_on_first_error) return false;
                  ok = false;
                  continue;
              }
          }
      }

      // Save the line
      if ( (line_num == 1) && (m_first_line_is_header)) out_data.m_header = split_line;
      else out_data.m_datas.append(split_line);

      // Limit the number of lines parsed
      if ( (m_max_lines_to_parse!= -1) && (line_num >= m_max_lines_to_parse) ) {
          break;
      }
    }
    if (out_error_msg && (out_error_msg->size() > 0)) ok = false;
    return ok;
}
