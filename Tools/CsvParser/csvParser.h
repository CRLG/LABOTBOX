#ifndef _CSVPARSER_H_
#define _CSVPARSER_H_

#include <QStringList>

#include "csvData.h"

/* Configurable parser for CSV file
 *
 * - Basic example :
 *
 * #include "csvParser.h"
 *  ...
 * QString pathfileName = "path and name of csv file...";
 * csvData data;
 * csvParser parser;
 * parser.parse(pathfileName, data);        // after parsing, "data" is filled with file content
 * qDebug() << data.m_header;               // display columnn description if exists
 * qDebug() << data.getColumnsCount();      // display the number of columns detected
 * qDebug() << data.getLinesCount();        // display the number of lines detected
 * for (int line=0; line<data.m_datas.size(); line++) {
 *     QStringList data_line = data.m_datas.at(line);
 *     for (int column=0; column<data_line.size(); column++) {
 *       qDebug() << data_line.at(column);
 *     }
 * }
 *
 *
 * - Example using some options and with error and warning messages handling
 *
 * csvData data;
 * csvParser parser;
 * QStringList error_msg, warn_msg;
 *
 * parser.enableEmptyCells(true);
 * parser.setMinimumNumberOfExpectedColums(4);
 * parser.setMaxLinesToParse(300);
 * parser.setSeparator(",");
 * bool status = parser.parse(pathfileName,data, &error_msg, &warn_msg);
 * if (status) {
 *     if (warn_msg.size() > 0) {
 *         qDebug() << "!! Warnings during parsing";
 *         foreach (QString msg, warn_msg) {
 *             qDebug() << "  -" << msg;
 *         }
 *     }
 *     else {
 *         qDebug() << "Parsing OK";
 *     }
 *     data.debug();
 * }
 * else {
 *     if (error_msg.size() > 0) {
 *         qDebug() << "!! ERRORS during parsing";
 *         foreach (QString msg, error_msg) {
 *             qDebug() << "  -" << msg;
 *         }
 *     }
 * }
*/

class csvParser
{
public:
    csvParser();
    ~csvParser();

    //! Parse the file
    //! Return true if non error occurs during parsing
    bool parse(const QString& pathfilename, csvData& out_data, QStringList *out_error_msg=nullptr, QStringList *out_warn_msg=nullptr);

    // Parsing options
    void setNumberOfFirstLinesToIgnore(long num);
    void setMaxLinesToParse(long max);
    void setNumberOfExpectedColums(long num);
    void setMinimumNumberOfExpectedColums(long num);
    void setMaximumNumberOfExpectedColums(long num);
    void enableEmptyCells(bool state);
    void setSeparator(QString sep);
    void firstLineIsHeader(bool state);
    void simplifieCells(bool state);
    void stopOnFirstErrorDetected(bool state);

protected:
    long m_ignore_first_n_lines;    //! Ignore first "n" lines
    long m_max_lines_to_parse;       //! Limit the number of line parsed (special value "-1" : do not limit)
    long m_num_of_columns_expected; //! The number of columns can be known by advance and checked during parsing(special value -1 = unknow)
    long m_min_of_columns_expected; //! The minimum number of columns expected and checked during parsing(special value -1 = unknow)
    long m_max_of_columns_expected; //! The maximum number of columns expected and checked during parsing(special value -1 = unknow)
    bool m_enable_empty_cells;      //! Indicate if a cell is allowed to be empty
    QString m_separator;            //! csv separator string
    bool m_first_line_is_header;    //! Indicate the first line is columns description
    bool m_simplifie_cells;         //! The cell will be simplified during parsing
    bool m_stop_on_first_error;     //! Stop parsing when first error occurs
};

#endif // _CSVPARSER_H_
