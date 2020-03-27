#ifndef _DOC_DESIGNER_H_
#define _DOC_DESIGNER_H_

#include "htmltexteditor.h"

/*
  Classe d'aide à la création de documents.
  Cette classe permet de créer des titres, des chapitres, des tableaux, du texte, du texte sur puces, des images.
  La police du texte peut être sélectionnée (taille, couleur, gras, ...), l'alignement des objets également.
  Le document est construit au fur et à mesure et peut être visualisé, exporté en html, exporté en PDF.

  Les images peuvent provenir soit de fichiers, soit de widgets Qt. Il est ainsi possible d'inclure dans le document
   la copie d'écran d'une fenêtre affichée à l'utilisateur.

  - Cas particulier des tableaux
  Un tableau est vu comme un QVector de QStringList où :
    * Chaque élément du QStringList représente une colonne
    * Chaque élément du QVector représente une ligne
  Les cellules peuvent être remplies avec du texte simple.
  Il est possible d'insérer dans une cellule un objet DocDesigner, contenant lui même des images, des puces, du texte formaté, ...
  Cela permet de créer des cellules complexes.

  - Cas des images
  Les images sont converties et embarquées directement dans le document (pas de référence à un fichier externe)

  - Utilisation :

      DocDesigner m_doc_designer;
      m_doc_designer.show();

      m_doc_designer.setAlign(Qt::AlignHCenter);
      m_doc_designer.appendPicture(":/icons/bloc_note.png", 2.5);

      m_doc_designer.appendDocTitle("Titre du document");
      m_doc_designer.setAlign(Qt::AlignRight);
      m_doc_designer.appendText("Ecrit par Toto, le xx/xx/xx");

      m_doc_designer.appendChapterTitle(1, "Nom du chapitre 1");
      m_doc_designer.appendText("Du texte juste en dessous du Chapitre 1");

      m_doc_designer.appendCRLF();
      m_doc_designer.appendChapterTitle(2, "Nom du chapitre 1.1");
      m_doc_designer.setFont("Courier", 14, true, true, true, QColor(Qt::darkRed));
      m_doc_designer.appendText("Du texte en rouge juste en dessous du Chapitre 1.1");

      m_doc_designer.appendCRLF();
      m_doc_designer.appendChapterTitle(2, "Nom du chapitre 1.2");
      m_doc_designer.setDefaultFont();
      m_doc_designer.appendText("Du texte normal juste en dessous du Chapitre 1.2");
      m_doc_designer.appendCRLF();
      m_doc_designer.appendText("Une ligne vide et du texte normal");

      m_doc_designer.appendCRLF();
      m_doc_designer.appendChapterTitle(1, "Nom du chapitre 2");
      m_doc_designer.appendText("Du texte juste en dessous du Chapitre 2");


      m_doc_designer.appendCRLF();
      m_doc_designer.appendChapterTitle(1, "Nom du chapitre 3");
      m_doc_designer.appendText("Une liste à puces");
      QStringList lst;
      lst << "Bonjour" << "Coucou" << "Hello";
      m_doc_designer.appendBulletList(lst);
      m_doc_designer.appendText("Du texte normal juste en dessous de la liste à puces", true);
      m_doc_designer.appendText("La même liste à puce mais en plus gros et en couleur !", true);
      m_doc_designer.setFont(m_doc_designer.getDefaultFamilyFont(), 24, true, true, true, Qt::darkBlue);
      m_doc_designer.appendBulletList(lst);


      m_doc_designer.appendCRLF();
      m_doc_designer.appendChapterTitle(1, "Nom du chapitre 4");
      m_doc_designer.appendText("Une belle image redimensionnée et centrée");

      m_doc_designer.setAlign(Qt::AlignHCenter);  // Centre l'image sur la page
      m_doc_designer.appendPicture(":/icons/bloc_note.png", 0.6);
      m_doc_designer.appendText("Image 1");

      m_doc_designer.appendCRLF();
      m_doc_designer.appendPicture(":/icons/bloc_note.png", 0.6, false);
      m_doc_designer.appendPicture(":/icons/bloc_note.png", 0.6);
      m_doc_designer.appendText("Deux images côtes à côtes");

      m_doc_designer.appendCRLF();
      m_doc_designer.appendChapterTitle(1, "Nom du chapitre 5");
      m_doc_designer.appendText("Dans ce chapitre, il va y avoir un tableau avec du texte simple et composé : 3 lignes / 4 colonnes");

      DocDesigner doc;  // document intermédiaire
      doc.setFont(doc.getDefaultFamilyFont(), 12, true, true, false, QColor(Qt::red), Qt::AlignHCenter);
      doc.appendText("Hello");
      doc.setFont("Courier", 14, true, false, true, QColor(Qt::darkGreen));
      doc.appendText("Du texte en vert");
      doc.setFont("Courier", 8, true, false, true, QColor(Qt::red), Qt::AlignHCenter);
      doc.appendText("Du texte en rouge");
      doc.setDefaultFont();
      doc.appendBulletList(lst);
      doc.setAlign(Qt::AlignHCenter);  // Centre l'image
      doc.appendPicture(":/icons/bloc_note.png", 0.5);

      DocDesigner doc2;  // document intermédiaire
      doc2.setFont(doc2.getDefaultFamilyFont(), 24, true, true, false, QColor(Qt::red), Qt::AlignHCenter);
      doc2.appendText("KO");

      QStringList l1, l2, l3;
      l1 << "L1/C1" << "L1/C2"              << doc2.getHtml()   << "L3/C4";
      l2 << "L2/C1" << doc.getHtml()        << "L2/C3"          << "L2/C4";
      l3 << "L3/C1" << "Plein de texte ici" << doc.getHtml()    << "L3/C4";
      QVector<QStringList>lines;
      lines << l1 << l2 << l3;
      m_doc_designer.appendTable(lines);


      m_doc_designer.exportToPdf("exemple_pdf");

  - Limitations :
  Pour les images, il peut y avoir un effet de compression sur l'affichage, que l'on ne retrouve pas dans le document généré au format html ou pdf.
*/

class DocDesigner : public HtmlTextEditor
{
    Q_OBJECT
public:
    explicit DocDesigner(QWidget *parent = 0);

public slots:
    void setDefaultFont();
    void setDefaultFont(QString family, qreal size, bool bold, bool italic, bool underline, QColor color=Qt::black, Qt::Alignment align=Qt::AlignLeft | Qt::AlignAbsolute);
    QString getDefaultFamilyFont();
    void setFont(QString family, qreal size, bool bold, bool italic, bool underline, QColor color=Qt::black, Qt::Alignment align=Qt::AlignLeft | Qt::AlignAbsolute);
    void setAlign(Qt::Alignment align);

    void appendText(QString text, bool final_crlf=true);
    void appendCRLF(unsigned int number=1);
    void appendDocTitle(QString text);
    void appendChapterTitle(int level, QString title, bool final_crlf=false);
    void appendBulletList(QStringList lst, QTextListFormat::Style style=QTextListFormat::ListDisc);
    void appendTable(QVector<QStringList> rows);
    void appendPicture(QString pathfilename, double scale=1.0, bool final_crlf=true);
    void appendPicture(const QImage& image, double scale=1.0, bool final_crlf=true);
    void appendWidget(QWidget* wdgt, double scale=1.0, bool final_crlf=true);

    void exportToPdf(QString pathfilename);
    //void exportToHtml(QString pathfilename);

    QString getHtml();

private :
    int m_current_chapter_level_1;
    int m_current_chapter_level_2;
    int m_current_chapter_level_3;

    QString         m_default_font_family;
    qreal           m_default_font_size;
    bool            m_default_font_bold;
    bool            m_default_font_italic;
    bool            m_default_font_underline;
    QColor          m_default_font_color;
    Qt::Alignment   m_default_font_align;
};

#endif // _DOC_DESIGNER_H_
