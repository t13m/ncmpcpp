/***************************************************************************
 *   Copyright (C) 2008-2012 by Andrzej Rybczak                            *
 *   electricityispower@gmail.com                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "global.h"
#include "song_info.h"
#include "tag_editor.h"

using Global::MainHeight;
using Global::MainStartY;

SongInfo *mySongInfo = new SongInfo;

const SongInfo::Metadata SongInfo::Tags[] =
{
 { "Title",        &MPD::Song::getTitle,       &MPD::MutableSong::setTitle       },
 { "Artist",       &MPD::Song::getArtist,      &MPD::MutableSong::setArtist      },
 { "Album Artist", &MPD::Song::getAlbumArtist, &MPD::MutableSong::setAlbumArtist },
 { "Album",        &MPD::Song::getAlbum,       &MPD::MutableSong::setAlbum       },
 { "Date",         &MPD::Song::getDate,        &MPD::MutableSong::setDate        },
 { "Track",        &MPD::Song::getTrack,       &MPD::MutableSong::setTrack       },
 { "Genre",        &MPD::Song::getGenre,       &MPD::MutableSong::setGenre       },
 { "Composer",     &MPD::Song::getComposer,    &MPD::MutableSong::setComposer    },
 { "Performer",    &MPD::Song::getPerformer,   &MPD::MutableSong::setPerformer   },
 { "Disc",         &MPD::Song::getDisc,        &MPD::MutableSong::setDisc        },
 { "Comment",      &MPD::Song::getComment,     &MPD::MutableSong::setComment     },
 { 0,              0,                          0                                  }
};

void SongInfo::Init()
{
	w = new Scrollpad(0, MainStartY, COLS, MainHeight, "", Config.main_color, brNone);
	isInitialized = 1;
}

void SongInfo::Resize()
{
	size_t x_offset, width;
	GetWindowResizeParams(x_offset, width);
	w->Resize(width, MainHeight);
	w->MoveTo(x_offset, MainStartY);
	hasToBeResized = 0;
}

std::basic_string<my_char_t> SongInfo::Title()
{
	return U("Song info");
}

void SongInfo::SwitchTo()
{
	using Global::myScreen;
	using Global::myOldScreen;
	using Global::myLockedScreen;
	
	if (myScreen == this)
		return myOldScreen->SwitchTo();
	
	if (!isInitialized)
		Init();
	
	if (myLockedScreen)
		UpdateInactiveScreen(this);
	
	auto hs = dynamic_cast<HasSongs *>(myScreen);
	if (!hs)
		return;
	auto s = hs->currentSong();
	if (!s)
		return;
	
	if (hasToBeResized || myLockedScreen)
		Resize();
	
	myOldScreen = myScreen;
	myScreen = this;
	
	Global::RedrawHeader = true;
	
	w->Clear();
	w->Reset();
	PrepareSong(*s);
	w->Flush();
}

void SongInfo::PrepareSong(MPD::Song &s)
{
#	ifdef HAVE_TAGLIB_H
	std::string path_to_file;
	if (s.isFromDatabase())
		path_to_file += Config.mpd_music_dir;
	path_to_file += s.getURI();
	TagLib::FileRef f(path_to_file.c_str());
#	endif // HAVE_TAGLIB_H
	
	*w << fmtBold << Config.color1 << U("Filename: ") << fmtBoldEnd << Config.color2 << s.getName() << '\n' << clEnd;
	*w << fmtBold << U("Directory: ") << fmtBoldEnd << Config.color2;
	ShowTag(*w, s.getDirectory());
	*w << U("\n\n") << clEnd;
	*w << fmtBold << U("Length: ") << fmtBoldEnd << Config.color2 << s.getLength() << '\n' << clEnd;
#	ifdef HAVE_TAGLIB_H
	if (!f.isNull())
	{
		*w << fmtBold << U("Bitrate: ") << fmtBoldEnd << Config.color2 << f.audioProperties()->bitrate() << U(" kbps\n") << clEnd;
		*w << fmtBold << U("Sample rate: ") << fmtBoldEnd << Config.color2 << f.audioProperties()->sampleRate() << U(" Hz\n") << clEnd;
		*w << fmtBold << U("Channels: ") << fmtBoldEnd << Config.color2 << (f.audioProperties()->channels() == 1 ? U("Mono") : U("Stereo")) << '\n' << clDefault;
	}
#	endif // HAVE_TAGLIB_H
	*w << clDefault;
	
	for (const Metadata *m = Tags; m->Name; ++m)
	{
		*w << fmtBold << '\n' << TO_WSTRING(m->Name) << U(": ") << fmtBoldEnd;
		ShowTag(*w, s.getTags(m->Get));
	}
}
