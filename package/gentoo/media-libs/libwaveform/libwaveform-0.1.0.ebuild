# Copyright 1999-2006 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

RESTRICT="nomirror"
DESCRIPTION="Libwaveform provides efficient display of audio waveforms for Gtk applications."
HOMEPAGE="http://ayyi.org/"
SRC_URI="http://ayyi.org/files/${P}.tar.gz"
RESTRICT="nomirror"
IUSE="ffmpeg sndfile"

LICENSE="GPL-3"
KEYWORDS="amd64 x86"
SLOT="0"

DEPEND="sndfile? ( >=media-libs/libsndfile-1.0.10 )
	ffmpeg? ( virtual/ffmpeg )
	>=x11-libs/gtk+-2.6
	media-libs/graphene"

src_install() {
	make DESTDIR=${D} install || die
	dodoc NEWS ChangeLog
}

