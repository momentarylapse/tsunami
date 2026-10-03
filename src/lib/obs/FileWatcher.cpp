//
// Created by michi on 9/28/26.
//

#include "FileWatcher.h"
#if defined(OS_LINUX)
#include <cstdio>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#endif

namespace obs {


FileWatcher::FileWatcher() {
#if defined(OS_LINUX)
	epoll_fd = epoll_create(1);
	if (epoll_fd < 0) {
		perror("Error while creating epoll instance");
		return;
	}

	inotify_fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
	if (inotify_fd < 0) {
		perror("Error while creating inotify instance");
		return;
	}

	epoll_event ev{.events = EPOLLIN};
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &ev) < 0) {
		perror("Error while adding inotify fd to epoll");
		return;
	}
#endif
}

FileWatcher::~FileWatcher() {
#if defined(OS_LINUX)
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, inotify_fd, NULL) < 0) {
		perror("Error while removing inotify fd from epoll");
		return;
	}

	/*if (inotify_rm_watch(inotify_fd, wd) < 0) {
		perror("Error while removing inotify watch descriptor");
		return;
	}*/

	if (close(inotify_fd) < 0) {
		perror("Error while destroying inotify instance");
		return;
	}

	if (close(epoll_fd) < 0) {
		perror("Error while destroying epoll instance");
		return;
	}
#endif
}

void FileWatcher::add_file(const Path &filename) {
#if defined(OS_LINUX)
	int wd = inotify_add_watch(inotify_fd, filename.c_str(), IN_ALL_EVENTS);
	if (wd < 0) {
		perror("Error while registering inotify watch");
		return;
	}
	items.add({wd, filename});
#endif
}

void FileWatcher::iterate() {
#if defined(OS_LINUX)
	while (true) {
		struct epoll_event event = {};
		if (epoll_wait(epoll_fd, &event, 1, 0) < 0) {
			perror("Error while waiting for IO events");
			return;
		}

		struct inotify_event in_event;
		ssize_t const read_bytes = read(inotify_fd, &in_event, sizeof(in_event));
		if (read_bytes < 0) {
			//perror("Error while reading event data");
			return;
		}
		if (read_bytes != sizeof(in_event)) {
			break;
		}

		if (in_event.mask == IN_CLOSE_WRITE) {
			//printf("File event %d on wd %d\n", in_event.mask, in_event.wd);
			for (const auto& i: items)
				if (i.w == in_event.wd)
					out_edited(i.filename);
		}
	}
#endif
}
} // obs