#include "FileDiffWidget.h"

#include <GitHistory.h>
#include <FileDiffView.h>
#include <FileDiffHighlighter.h>
#include <CommitInfo.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollBar>

FileDiffWidget::FileDiffWidget(const QSharedPointer<GitBase> &git, QWidget *parent)
   : QFrame(parent)
   , mGit(git)
   , mDiffView(new FileDiffView())
   , mGoPrevious(new QPushButton())
   , mGoNext(new QPushButton())

{
   setAttribute(Qt::WA_DeleteOnClose);

   mDiffHighlighter = new FileDiffHighlighter(mDiffView->document());

   mGoPrevious->setIcon(QIcon(":/icons/go_up"));
   mGoNext->setIcon(QIcon(":/icons/go_down"));

   const auto vLayout = new QHBoxLayout(this);
   vLayout->setContentsMargins(QMargins());
   vLayout->setSpacing(0);
   vLayout->addWidget(mDiffView);
}

void FileDiffWidget::clear()
{
   mDiffView->clear();
}

bool FileDiffWidget::reload()
{
   if (mCurrentSha == CommitInfo::ZERO_SHA)
      return configure(mCurrentSha, mPreviousSha, mCurrentFile);

   return false;
}

bool FileDiffWidget::configure(const QString &currentSha, const QString &previousSha, const QString &file)
{
   mCurrentFile = file;
   mCurrentSha = currentSha;
   mPreviousSha = previousSha;

   auto destFile = file;

   if (destFile.contains("-->"))
      destFile = destFile.split("--> ").last().split("(").first().trimmed();

   QScopedPointer<GitHistory> git(new GitHistory(mGit));
   auto text = git->getFileDiff(currentSha == CommitInfo::ZERO_SHA ? QString() : currentSha, previousSha, destFile);
   auto lines = text.split("\n");

   for (auto i = 0; !lines.isEmpty() && i < 5; ++i)
      lines.takeFirst();

   if (!lines.isEmpty())
   {
      text = lines.join("\n");

      const auto pos = mDiffView->verticalScrollBar()->value();
      mDiffView->setPlainText(text);

      mRowIndex = 0;
      mDiffHighlighter->resetState();

      mDiffView->moveCursor(QTextCursor::Start);

      mDiffView->verticalScrollBar()->setValue(pos);

      return true;
   }

   return false;
}
