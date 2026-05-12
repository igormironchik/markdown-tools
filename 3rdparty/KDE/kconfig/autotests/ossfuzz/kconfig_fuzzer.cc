/*
 * SPDX-FileCopyrightText: 2026 Nicolas Fella <nicolas.fella@gmx.de>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QCoreApplication>
#include <QBuffer>

#include <kconfig.h>
#include <kconfiggroup.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {

  int argc = 0;
  QCoreApplication a(argc, nullptr);

  QByteArray input((const char *)data, size);

  auto buffer = std::make_shared<QBuffer>(&input);

  buffer->open(QIODevice::ReadOnly);

  KConfig config(buffer);

  const auto groups = config.groupList();
  for (const auto &groupName : groups) {
    KConfigGroup group = config.group(groupName);
    group.entryMap();
  }

  return 0;
}
