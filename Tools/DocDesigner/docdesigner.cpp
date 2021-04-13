#include <QMainWindow>
#include <QComboBox>
#include <QAction>
#include <QStatusBar>
#include <QPainter>
#include <QTextDocumentWriter>
#include <QPrinter>
#include <QFileInfo>
#include <QTextList>
#include <QDebug>
#include <QBuffer>
#include <textedit.h>

#include "docdesigner.h"

DocDesigner::DocDesigner(QObject*parent)
    : QTextDocument(parent),
      m_current_chapter_level_1(0),
      m_current_chapter_level_2(0),
      m_current_chapter_level_3(0),
      m_default_font("DejaVu Sans", 10, false, false, false, QColor(Qt::black), Qt::AlignLeft, 0),
      m_current_font("DejaVu Sans", 10, false, false, false, QColor(Qt::black), Qt::AlignLeft, 0),
      m_text_cursor(this)
{
    setDefaultFont();
}

// ________________________________________________________
QString DocDesigner::getDefaultFamilyFont()
{
    return m_default_font.m_family;
}

// ________________________________________________________
void DocDesigner::setDefaultFont()
{

    textFamily(m_default_font.m_family);
    textSize(m_default_font.m_size);
    textBold(m_default_font.m_bold);
    textItalic(m_default_font.m_italic);
    textUnderline(m_default_font.m_underline);
    textColor(m_default_font.m_color);
    textAlign(m_default_font.m_align);
    textIndent(m_default_font.m_indent);
    m_current_font = m_default_font;
}

// ________________________________________________________
void DocDesigner::setDefaultFont(const DocFont& font)
{
    m_default_font.m_family     = font.m_family;
    m_default_font.m_size       = font.m_size;
    m_default_font.m_bold       = font.m_bold;
    m_default_font.m_italic     = font.m_italic;
    m_default_font.m_underline  = font.m_underline;
    m_default_font.m_color      = font.m_color;
    m_default_font.m_align      = font.m_align;
    m_current_font.m_indent     = font.m_indent;
    setDefaultFont();
}

// ________________________________________________________
void DocDesigner::setFont(const DocFont& font)
{

    if (font.m_family != "") textFamily(font.m_family);
    textSize(font.m_size);
    textBold(font.m_bold);
    textItalic(font.m_italic);
    textUnderline(font.m_underline);
    textColor(font.m_color);
    textAlign(font.m_align);
    textIndent(font.m_indent);

    if (font.m_family != "") m_current_font.m_family = font.m_family;
    m_current_font.m_size       = font.m_size;
    m_current_font.m_bold       = font.m_bold;
    m_current_font.m_italic     = font.m_italic;
    m_current_font.m_underline  = font.m_underline;
    m_current_font.m_color      = font.m_color;
    m_current_font.m_align      = font.m_align;
    m_current_font.m_indent     = font.m_indent;
}

// ________________________________________________________
QString DocDesigner::getFormatedText(const QString& text, const DocFont& font, bool final_crlf)
{
    DocDesigner doc;
    doc.setFont(font);
    doc.appendText(text, final_crlf);
    return doc.getHtml();
}

// ________________________________________________________
void DocDesigner::setAlign(Qt::Alignment align)
{
    textAlign(align);
    m_current_font.m_align = align;
}

// ________________________________________________________
void DocDesigner::setIndent(int indent)
{
    textIndent(indent);
    m_current_font.m_indent = indent;
}

// ________________________________________________________
void DocDesigner::appendText(QString text, bool final_crlf)
{
    m_text_cursor.insertText(text);
    if (final_crlf) m_text_cursor.insertText("\n");
}

// ________________________________________________________
void DocDesigner::appendText(QString text, const DocFont& font, bool final_crlf)
{

    m_text_cursor.insertHtml(getFormatedText(text, font));
    if (final_crlf) appendCRLF(1);
}

// ________________________________________________________
void DocDesigner::appendCRLF(unsigned int number)
{
    for (unsigned int i=0; i<number; i++) {
        m_text_cursor.insertText("\n");
    }
}

// ________________________________________________________
void DocDesigner::appendDocTitle(QString text)
{
     setFont(DocFont(m_default_font.m_family,
                    24,     // size
                    true,   // bold
                    false,  // italic
                    true,   // underline
                    QColor("darkBlue"),
                    Qt::AlignHCenter));
    appendText(text, false);
    if (!text.endsWith("\n")) {
         appendCRLF();
    }
    setDefaultFont();
}

// ________________________________________________________
void DocDesigner::appendChapterTitle(int level, QString title, bool before_crlf, bool final_crlf)
{
    QString txt;

    setDefaultFont();
    if (before_crlf) appendCRLF();
    switch(level) {
    // 1. Titre Niveau 1
    case 0:
    case 1:
        textBold(true);
        textUnderline(true);
        textSize(14);
        m_current_chapter_level_1++;
        m_current_chapter_level_2 = 0;
        m_current_chapter_level_3 = 0;

        txt += QString::number(m_current_chapter_level_1);
        txt += ". ";
        txt += title;
        appendText(txt, true);
        break;
    // 1.1 Titre Niveau 2
    case 2:
        textUnderline(false);
        textBold(true);
        textUnderline(true);
        textSize(12);
        textIndent(1);
        m_current_chapter_level_2++;
        m_current_chapter_level_3 = 0;
        txt += QString::number(m_current_chapter_level_1);
        txt += ".";
        txt += QString::number(m_current_chapter_level_2);
        txt += ". ";
        txt += title;
        appendText(txt, true);
        break;
    // 1.1.1 Titre Niveau 3
    case 3:
        textUnderline(false);
        textBold(true);
        textUnderline(true);
        textSize(11);
        textIndent(2);
        m_current_chapter_level_3++;
        txt += QString::number(m_current_chapter_level_1);
        txt += ".";
        txt += QString::number(m_current_chapter_level_2);
        txt += ".";
        txt += QString::number(m_current_chapter_level_3);
        txt += ". ";
        txt += title;
        appendText(txt, true);
        break;
     default :
        txt += title;
        appendText(txt, true);
        break;
    }
    if (final_crlf) appendCRLF();
    setDefaultFont();
}

// ________________________________________________________
void DocDesigner::appendBulletList(QStringList lst, QString bullet_char, int indent)
{
    int current_indent = m_text_cursor.blockFormat().indent();
    if (indent < 0) setIndent(current_indent + 1);  // indentation automatique : idente d'un cran
    else setIndent(indent);                         // indentation manuelle
    for (int i=0; i<lst.size(); i++) {
        appendText(bullet_char + " " + lst.at(i));
    }
    setIndent(current_indent); // restitue le niveau d'indentation initial
}


// ________________________________________________________
void DocDesigner::appendNumList(QStringList lst, int start_num, int indent)
{
    int current_indent = m_text_cursor.blockFormat().indent();
    if (indent < 0) setIndent(current_indent + 1);  // indentation automatique : idente d'un cran
    else setIndent(indent);                         // indentation manuelle
    for (int i=0; i<lst.size(); i++) {
        appendText(QString::number(start_num + i) + ". " + lst.at(i));
    }
    setIndent(current_indent); // restitue le niveau d'indentation initial
}

// ______________________________________________________
//! BUG : si la cellule est composée de texte brut (et pas de html),
//!   la police de caractère de cette cellule est une police par défaut.
void DocDesigner::appendTable(QVector<QStringList> rows)
{
    QString html;

    if (rows.size() == 0) return;

    html += "<table border =\"10\">";
    for (int row=0; row<rows.size(); row++) {
        QStringList row_str = rows.at(row);
        html += "<tr>";
        for (int column=0; column<row_str.size(); column++) {
            html += "<td>";
            html += row_str.at(column);
            html += "</td>";
        }
        html += "</tr>";
    }
    html += "</table>";
    m_text_cursor.insertHtml(html);
    setFont(m_current_font);  // Restitue la dernière police de caractère utilisée qui a été perdue lors de l'insertion du tableau

}

// ______________________________________________________
void DocDesigner::appendPicture(const QImage& image, double width, double height, bool final_crlf, Qt::Alignment align)
{
    Qt::Alignment save_align = m_text_cursor.blockFormat().alignment();
    textAlign(align);

    dropImage(image, "JPG", width, height);

     if (final_crlf) appendCRLF();

    textAlign(save_align);  // restitue le mode d'alignement changé juste
}


// ______________________________________________________
void DocDesigner::appendPicture(QString pathfilename, double width, double height, bool final_crlf, Qt::Alignment align)
{
    QImage image(pathfilename);
    appendPicture(image, width, height, final_crlf, align);
}

// ______________________________________________________
void DocDesigner::appendWidget(QWidget* wdgt, double width, double height, bool final_crlf, Qt::Alignment align)
{
    QImage image(wdgt->size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    wdgt->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    appendPicture(image, width, height, final_crlf, align);
}

// ______________________________________________________
void DocDesigner::dropImage(const QImage& image, const QString& format, double width, double height)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, format.toLocal8Bit().data());
    buffer.close();
    QByteArray base64 = bytes.toBase64();
    QByteArray base64l;
    for (int i=0; i<base64.size(); i++) {
        base64l.append(base64[i]);
        if (i%80 == 0) {
            base64l.append("\n");
        }
    }

    QTextImageFormat imageFormat;
    if (width < 0)  imageFormat.setWidth(image.width());
    else            imageFormat.setWidth(width);
    if (height < 0) imageFormat.setHeight(image.height());
    else            imageFormat.setHeight(height );
    imageFormat.setName   ( QString("data:image/%1;base64,%2")
                            .arg(QString("%1.%2").arg(rand()).arg(format))
                            .arg(base64l.data())
                            );
    m_text_cursor.insertImage(imageFormat);
}


// ______________________________________________________
void DocDesigner::appendDoc(DocDesigner *doc)
{
    appendHtml(doc->getHtml());
}

// ______________________________________________________
void DocDesigner::appendHtml(QString html)
{
    m_text_cursor.insertHtml(html);
    setFont(m_current_font);  // Restitue la dernière police de caractère utilisée qui a été perdue lors de l'insertion du tableau
}

// ________________________________________________________
QString DocDesigner::getHtml()
{
    return toHtml();
}

// ________________________________________________________
void DocDesigner::exportToPdf(QString pathfilename)
{
#ifndef QT_NO_PRINTER
    if (!pathfilename.isEmpty()) {
        if (QFileInfo(pathfilename).suffix().isEmpty())
            pathfilename.append(".pdf");
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(pathfilename);
        print(&printer);
    }
#endif
}

// ________________________________________________________
void DocDesigner::exportToHtml(QString pathfilename)
{
    QTextDocumentWriter writer(pathfilename);
    if (! (pathfilename.endsWith(".htm") || (pathfilename.endsWith(".html")))) {
            pathfilename.append(".html");
    }
    writer.setFormat("html");
    writer.write(this);
}

// ________________________________________________________
//! BUG : les images ne sont pas présentes dans le document ODF généré
void DocDesigner::exportToOdf(QString pathfilename)
{

    QTextDocumentWriter writer(pathfilename);
    if (!pathfilename.endsWith(".odt")) {
        pathfilename.append(".odt");
    }
    writer.setFormat("odf");
    writer.write(this);
}

// ________________________________________________________
void DocDesigner::textColor(QColor col)
{
    QTextCharFormat fmt;
    fmt.setForeground(col);
    m_text_cursor.mergeCharFormat(fmt);
}

// ________________________________________________________
void DocDesigner::textBold(bool bold)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    m_text_cursor.mergeCharFormat(fmt);
}

// ________________________________________________________
void DocDesigner::textUnderline(bool underline)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(underline);
    m_text_cursor.mergeCharFormat(fmt);
}

// ________________________________________________________
void DocDesigner::textItalic(bool italic)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(italic);
    m_text_cursor.mergeCharFormat(fmt);
}

// ________________________________________________________
void DocDesigner::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    m_text_cursor.mergeCharFormat(fmt);
}

// ________________________________________________________
void DocDesigner::textSize(qreal pointSize)
{
    if (pointSize > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        m_text_cursor.mergeCharFormat(fmt);
    }
}

// ________________________________________________________
void DocDesigner::textAlign(Qt::Alignment align)
{
    QTextBlockFormat blockFmt = m_text_cursor.blockFormat();
    blockFmt.setAlignment(align);
    m_text_cursor.mergeBlockFormat(blockFmt);
}

// ________________________________________________________
void DocDesigner::textIndent(int indent)
{
    QTextBlockFormat blockFmt = m_text_cursor.blockFormat();
    blockFmt.setIndent(indent);
    m_text_cursor.mergeBlockFormat(blockFmt);
}
