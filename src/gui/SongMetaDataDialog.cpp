/*
 * SongMetaDataDialog.cpp - dialog for setting song properties/tags/metadata
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "SongMetaDataDialog.h"

//#include <QFileInfo>
//#include <QDir>
//#include <QMessageBox>
//#include <QDebug>

#include "InstrumentFunction.h"
#include "Song.h"
#include "lmmsversion.h"
//#include "GuiApplication.h"
//#include "MainWindow.h"
//#include "OutputSettings.h"

SongMetaDataDialog::SongMetaDataDialog(QWidget* _parent) :
      QDialog(_parent), Ui::SongMetaDataDialog()
{
    setupUi(this);
    setWindowTitle(tr("Song Properties"));

    // tab 1
    Song* song = Engine::getSong();
    value_Tempo->setText(QString("%1 bpm").arg(song->getTempo()));
    value_TimeSignature->setText(
            QString("%1/%2")
                    .arg(song->getTimeSigModel().getNumerator())
                    .arg(song->getTimeSigModel().getDenominator()));

    value_Created->setText(song->songMetaData("Created"));
    value_Modified->setText(song->songMetaData("Modified"));

    qint64 cwt = song->songMetaData("CumulativeWorkTime").toULongLong();
    value_CumulativeWorkTime->setText(QString("%1h %2m %3s")
                                              .arg(cwt / 3600)
                                              .arg((cwt / 60) % 60)
                                              .arg(cwt % 60));

    value_Software->setText(QString("LSMM %1").arg(LMMS_VERSION));

    for(int i = 0; i < 17; i++)
        value_Structure->addItem(STRUCTURES[i]);

    value_Structure->setCurrentText(song->songMetaData("Structure"));

    for(int key = 0; key < KeysPerOctave; key++)
        value_Root->addItem(Note::findNoteName(key));
    value_Root->setCurrentText(song->songMetaData("Root"));

    /*
    const InstrumentFunctionNoteStacking::ChordTable& chord_table
            = InstrumentFunctionNoteStacking::ChordTable::getInstance();

    value_Mode->addItem(tr("No mode"));
    for(const InstrumentFunctionNoteStacking::Chord& chord: chord_table)
        if(chord.isMode())
            value_Mode->addItem(chord.getName());
    */

    value_Mode->addItem(tr("No mode"));
    value_Mode->setCurrentIndex(0);
    ChordDef::map([this](auto chord) {
        if(chord.isMode())
            value_Mode->addItem(chord.name());
    });
    value_Mode->setCurrentText(song->songMetaData("Mode"));

    for(int genre = 0; genre <= 191; genre++)
        value_Genre->addItem(GENRES[genre]);

    // tab 2
    value_License->addItem("AL 2.0 (Artistic License)");
    value_License->addItem("CPL 1.0 (Common Public License)");
    value_License->addItem("CC0 (Creative Commons)");
    value_License->addItem("CC-BY 4.0 (Creative Commons)");
    // value_License->addItem("CC-BY-SA 4.0 (Creative Commons)");
    value_License->addItem("FDL 1.3+ (GNU Free Documentation License)");
    value_License->addItem("BSD (Simplified BSD License)");
    value_License->addItem("Green OpenMusic");

    value_SongTitle->setText(song->songMetaData("SongTitle"));
    value_Artist->setText(song->songMetaData("Artist"));
    value_AlbumTitle->setText(song->songMetaData("AlbumTitle"));
    value_TrackNumber->setText(song->songMetaData("TrackNumber"));
    value_ReleaseDate->setDate(QDate::fromString(
            song->songMetaData("ReleaseDate"), Qt::ISODate));
    value_IRCS->setText(song->songMetaData("IRCS"));
    value_Copyright->setText(song->songMetaData("Copyright"));
    value_License->setCurrentText(song->songMetaData("License"));
    value_Genre->setCurrentText(song->songMetaData("Genre"));
    value_Subgenre->setText(song->songMetaData("Subgenre"));

    // tab 3
    value_ArtistWebsite->setText(song->songMetaData("ArtistWebsite"));
    value_LabelWebsite->setText(song->songMetaData("LabelWebsite"));
    value_Amazon->setText(song->songMetaData("Amazon"));
    value_BandCamp->setText(song->songMetaData("BandCamp"));
    value_Clyp->setText(song->songMetaData("Clyp"));
    value_iTunes->setText(song->songMetaData("iTunes"));
    value_LMMS->setText(song->songMetaData("LSP"));
    value_Orfium->setText(song->songMetaData("Orfium"));
    value_SoundCloud->setText(song->songMetaData("SoundCloud"));
    value_Spotify->setText(song->songMetaData("Spotify"));
    value_YouTube->setText(song->songMetaData("YouTube"));
}

SongMetaDataDialog::~SongMetaDataDialog()
{
}

void SongMetaDataDialog::reject()
{
    QDialog::reject();
}

void SongMetaDataDialog::accept()
{
    // tab 1
    Song* song = Engine::getSong();
    song->setSongMetaData("Structure", value_Structure->currentText());
    song->setSongMetaData("Root", value_Root->currentText());
    song->setSongMetaData("Mode", value_Mode->currentText());

    // tab 2
    song->setSongMetaData("SongTitle", value_SongTitle->text());
    song->setSongMetaData("Artist", value_Artist->text());
    song->setSongMetaData("AlbumTitle", value_AlbumTitle->text());
    song->setSongMetaData("TrackNumber", value_TrackNumber->text());
    song->setSongMetaData("ReleaseDate",
                          value_ReleaseDate->date().toString(Qt::ISODate));
    song->setSongMetaData("IRCS", value_IRCS->text());
    song->setSongMetaData("Copyright", value_Copyright->text());
    song->setSongMetaData("License", value_License->currentText());
    song->setSongMetaData("Genre", value_Genre->currentText());
    song->setSongMetaData("Subgenre", value_Subgenre->text());

    // tab 3
    song->setSongMetaData("ArtistWebsite", value_ArtistWebsite->text());
    song->setSongMetaData("LabelWebsite", value_LabelWebsite->text());
    song->setSongMetaData("Amazon", value_Amazon->text());
    song->setSongMetaData("BandCamp", value_BandCamp->text());
    song->setSongMetaData("Clyp", value_Clyp->text());
    song->setSongMetaData("iTunes", value_iTunes->text());
    song->setSongMetaData("LMMS", value_LMMS->text());
    song->setSongMetaData("Orfium", value_Orfium->text());
    song->setSongMetaData("SoundCloud", value_SoundCloud->text());
    song->setSongMetaData("Spotify", value_Spotify->text());
    song->setSongMetaData("YouTube", value_YouTube->text());

    QDialog::accept();
}

void SongMetaDataDialog::closeEvent(QCloseEvent* _ce)
{
    QDialog::closeEvent(_ce);
}

const QString SongMetaDataDialog::GENRES[] = {
        "Blues",
        "Classic Rock",
        "Country",
        "Dance",
        "Disco",
        "Funk",
        "Grunge",
        "Hip-Hop",
        "Jazz",
        "Metal",
        "New Age",
        "Oldies",
        "Other",
        "Pop",
        "Rhythm and Blues",
        "Rap",
        "Reggae",
        "Rock",
        "Techno",
        "Industrial",
        "Alternative",
        "Ska",
        "Death Metal",
        "Pranks",
        "Soundtrack",
        "Euro-Techno",
        "Ambient",
        "Trip-Hop",
        "Vocal",
        "Jazz & Funk",
        "Fusion",
        "Trance",
        "Classical",
        "Instrumental",
        "Acid",
        "House",
        "Game",
        "Sound clip",
        "Gospel",
        "Noise",
        "Alternative Rock",
        "Bass",
        "Soul",
        "Punk",
        "Space",
        "Meditative",
        "Instrumental Pop",
        "Instrumental Rock",
        "Ethnic",
        "Gothic",
        "Darkwave",
        "Techno-Industrial",
        "Electronic",
        "Pop-Folk",
        "Eurodance",
        "Dream",
        "Southern Rock",
        "Comedy",
        "Cult",
        "Gangsta",
        "Top 40",
        "Christian Rap",
        "Pop/Funk",
        "Jungle music",
        "Native US",
        "Cabaret",
        "New Wave",
        "Psychedelic",
        "Rave",
        "Showtunes",
        "Trailer",
        "Lo-Fi",
        "Tribal",
        "Acid Punk",
        "Acid Jazz",
        "Polka",
        "Retro",
        "Musical",
        "Rock ’n’ Roll",
        "Hard Rock",
        "Folk",
        "Folk-Rock",
        "National Folk",
        "Swing",
        "Fast Fusion",
        "Bebop",
        "Latin",
        "Revival",
        "Celtic",
        "Bluegrass",
        "Avantgarde",
        "Gothic Rock",
        "Progressive Rock",
        "Psychedelic Rock",
        "Symphonic Rock",
        "Slow Rock",
        "Big Band",
        "Chorus",
        "Easy Listening",
        "Acoustic",
        "Humour",
        "Speech",
        "Chanson",
        "Opera",
        "Chamber Music",
        "Sonata",
        "Symphony",
        "Booty Bass",
        "Primus",
        "Porn Groove",
        "Satire",
        "Slow Jam",
        "Club",
        "Tango",
        "Samba",
        "Folklore",
        "Ballad",
        "Power Ballad",
        "Rhythmic Soul",
        "Freestyle",
        "Duet",
        "Punk Rock",
        "Drum Solo",
        "A cappella",
        "Euro-House",
        "Dance Hall",
        "Goa music",
        "Drum & Bass",
        "Club-House",
        "Hardcore Techno",
        "Terror",
        "Indie",
        "BritPop",
        "Negerpunk",
        "Polsk Punk",
        "Beat",
        "Christian Gangsta Rap",
        "Heavy Metal",
        "Black Metal",
        "Crossover",
        "Contemporary Christian",
        "Christian Rock",
        "Merengue",
        "Salsa",
        "Thrash Metal",
        "Anime",
        "Jpop",
        "Synthpop",
        "Abstract",
        "Art Rock",
        "Baroque",
        "Bhangra",
        "Big beat",
        "Breakbeat",
        "Chillout",
        "Downtempo",
        "Dub",
        "EBM",
        "Eclectic",
        "Electro",
        "Electroclash",
        "Emo",
        "Experimental",
        "Garage",
        "Global",
        "IDM",
        "Illbient",
        "Industro-Goth",
        "Jam Band",
        "Krautrock",
        "Leftfield",
        "Lounge",
        "Math Rock",
        "New Romantic",
        "Nu-Breakz",
        "Post-Punk",
        "Post-Rock",
        "Psytrance",
        "Shoegaze",
        "Space Rock",
        "Trop Rock",
        "World Music",
        "Neoclassical",
        "Audiobook",
        "Audio Theatre",
        "Neue Deutsche Welle",
        "Podcast",
        "Indie-Rock",
        "G-Funk",
        "Dubstep",
        "Garage Rock",
        "Psybient",
};

const QString SongMetaDataDialog::STRUCTURES[]
        = {"ABA (Rondo 1)",
           "ABACA (Rondo 2)",
           "ABACABA (Rondo 3)",
           "ABACADA (Rondo 4)",
           "AA'BA''CA'''BA'''' (Rondo 5)",
           "ABA'CA''B'A (Rondo 6)",
           "ABCD (Medley 1)",
           "AABBCCDD (Medley 2)",
           "AB (Binary 1)",
           "AABB (Binary 2)",
           "ABAFZA (Ostinato)",
           "AAA (Song 1)",
           "AAAAA (Song 2)",
           "AABA (Song 3)",
           "AABABA (Song 4)",
           "ABABABB (Song 5)",
           "AAB (Blues)"};

/*
  I Introduction a.k.a. Intro
  A Verse
  B Refrain
  R Pre-chorus / Rise / Climb
  H Chorus
  P Post-chorus
  C Bridge
  M Middle-Eight
  S Solo / Instrumental Break
  L Collision
  O CODA / Outro
*/
