//
// Created by michi on 9/28/26.
//

#pragma once

#include "Observable.h"
#include "../os/path.h"

namespace obs {

	class FileWatcher : public obs::Node<VirtualBase> {
	public:
		FileWatcher();
		~FileWatcher() override;
		void add_file(const Path& filename);
		void iterate();

		xsource<Path> out_edited{this, "edited"};

		int epoll_fd;
		int inotify_fd;
		struct Item {
			int w;
			Path filename;
		};
		Array<Item> items;
	};
} // obs

