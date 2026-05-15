/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJOB_H
#define KJOB_H

#include <QObject>
#include <QPair>
#include <kcoreaddons_export.h>
#include <memory>

class KJobUiDelegate;

class KJobPrivate;

/*!
 * \class KJob
 * \inmodule KCoreAddons
 *
 * \brief The base class for all jobs.
 *
 * For all jobs created in an application, the code looks like
 *
 * \code
 * void SomeClass::methodWithAsynchronousJobCall()
 * {
 *   KJob *job = someoperation(some parameters);
 *   connect(job, &KJob::result, this, &SomeClass::handleResult);
 *   job->start();
 * }
 * \endcode
 *   (other connects, specific to the job)
 *
 * And handleResult is usually at least:
 *
 * \code
 * void SomeClass::handleResult(KJob *job)
 * {
 *   if (job->error()) {
 *       doSomething();
 *   }
 * }
 * \endcode
 *
 * With the synchronous interface the code looks like
 *
 * \code
 * void SomeClass::methodWithSynchronousJobCall()
 * {
 *   KJob *job = someoperation( some parameters );
 *   if (!job->exec()) {
 *       // An error occurred
 *   } else {
 *       // Do something
 *   }
 * }
 * \endcode
 *
 * Subclasses must implement start(), which should trigger the execution of
 * the job (although the work should be done asynchronously). errorString()
 * should also be reimplemented by any subclasses that introduce new error
 * codes.
 *
 * \note KJob and its subclasses are meant to be used in a fire-and-forget way.
 * Jobs will delete themselves when they finish using deleteLater() (although
 * this behaviour can be changed), so a job instance will disappear after the
 * next event loop run.
 */
class KCOREADDONS_EXPORT KJob : public QObject
{
    Q_OBJECT

    /*!
     * \property KJob::error
     */
    Q_PROPERTY(int error READ error NOTIFY result)

    /*!
     * \property KJob::errorText
     */
    Q_PROPERTY(QString errorText READ errorText NOTIFY result)

    /*!
     * \property KJob::errorString
     */
    Q_PROPERTY(QString errorString READ errorString NOTIFY result)

    /*!
     * \property KJob::percent
     */
    Q_PROPERTY(ulong percent READ percent NOTIFY percentChanged) // KF6 TODO: make "int", is enough

    /*!
     * \property KJob::capabilities
     */
    Q_PROPERTY(Capabilities capabilities READ capabilities CONSTANT)

public:
    /*!
     * Describes the unit used in the methods that handle reporting the job progress info.
     * \sa totalAmount
     *
     * \value Bytes Directory and file sizes in bytes
     * \value Files The number of files handled by the job
     * \value Directories The number of directories handled by the job
     * \value [since 5.72] Items The number of items (e.g. both directories and files) handled by the job
     * \value [since 5.87] UnitsCount Used internally only, do not use
     */
    enum Unit {
        Bytes = 0,
        Files,
        Directories,
        Items,
        UnitsCount,
    };
    Q_ENUM(Unit)

    /*!
     * \value NoCapabilities None of the capabilities exist
     * \value Killable The job can be killed
     * \value Suspendable The job can be suspended
     */
    enum Capability {
        NoCapabilities = 0x0000,
        Killable = 0x0001,
        Suspendable = 0x0002,
    };
    Q_ENUM(Capability)

    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities)

    /*!
     * Creates a new KJob object.
     *
     * \a parent the parent QObject
     */
    explicit KJob(QObject *parent = nullptr);

    ~KJob() override;

    /*!
     * Attach a UI delegate to this job.
     *
     * If the job had another UI delegate, it's automatically deleted. Once
     * attached to the job, the UI delegate will be deleted with the job.
     *
     * \a delegate the new UI delegate to use
     *
     * \sa KJobUiDelegate
     */
    void setUiDelegate(KJobUiDelegate *delegate);

    /*!
     * Returns the delegate attached to this job, or \c nullptr if there's no such delegate
     */
    KJobUiDelegate *uiDelegate() const;

    /*!
     * Returns the capabilities that this job supports
     * \sa setCapabilities()
     */
    Capabilities capabilities() const;

    /*!
     * Returns if the job was suspended with the suspend() call.
     *
     * \sa suspend(), resume()
     */
    bool isSuspended() const;

    // TODO KF7 make it non-virtual and expose a doStart protected instead
    /*!
     * Starts the job asynchronously.
     *
     * When the job is finished, result() is emitted.
     *
     * Warning: Never implement any synchronous workload in this method. This method
     * should just trigger the job startup, not do any work itself. It is expected to
     * be non-blocking.
     *
     * This is the method all subclasses need to implement.
     * It should setup and trigger the workload of the job. It should not do any
     * work itself. This includes all signals and terminating the job, e.g. by
     * emitResult(). The workload, which could be another method of the
     * subclass, is to be triggered using the event loop, e.g. by code like:
     * \code
     * void ExampleJob::start()
     * {
     *  QTimer::singleShot(0, this, &ExampleJob::doWork);
     * }
     * \endcode
     */
    Q_SCRIPTABLE virtual void start() = 0;

    /*!
     * \value Quietly
     * \value EmitResult
     */
    enum KillVerbosity {
        Quietly,
        EmitResult,
    };
    Q_ENUM(KillVerbosity)

public Q_SLOTS:
    /*!
     * Aborts this job.
     *
     * This kills and deletes the job.
     *
     * \a verbosity if equals to EmitResult, Job will emit signal result
     * and ask uiserver to close the progress window.
     *
     * \a verbosity is set to EmitResult for subjobs. Whether applications
     * should call with Quietly or EmitResult depends on whether they rely
     * on result being emitted or not. Please notice that if \a verbosity is
     * set to Quietly, signal result will NOT be emitted.
     *
     * Returns \c true if the operation is supported and succeeded, false otherwise
     */
    bool kill(KJob::KillVerbosity verbosity = KJob::Quietly);

    /*!
     * Suspends this job.
     *
     * The job should be kept in a state in which it is possible to resume it.
     *
     * Returns \c true if the operation is supported and succeeded, \c false otherwise
     */
    bool suspend();

    /*!
     * Resumes this job.
     *
     * Returns \c true if the operation is supported and succeeded, \c false otherwise
     */
    bool resume();

protected:
    /*!
     * Aborts this job quietly.
     *
     * This simply kills the job, no error reporting or job deletion should be involved.
     *
     * Returns \c true if the operation is supported and succeeded, \c false otherwise
     */
    virtual bool doKill();

    /*!
     * Suspends this job.
     *
     * Returns \c true if the operation is supported and succeeded, \c false otherwise
     */
    virtual bool doSuspend();

    /*!
     * Resumes this job.
     *
     * Returns \c true if the operation is supported and succeeded, \c false otherwise
     */
    virtual bool doResume();

    /*!
     * Sets the capabilities for this job.
     *
     * \a capabilities are the capabilities supported by this job
     *
     * \sa capabilities()
     */
    void setCapabilities(Capabilities capabilities);

public:
    /*!
     * Executes the job synchronously.
     *
     * This will start a nested QEventLoop internally. Nested event loop can be dangerous and
     * can have unintended side effects, you should avoid calling exec() whenever you can and use the
     * asynchronous interface of KJob instead.
     *
     * Should you indeed call this method, you need to make sure that all callers are reentrant,
     * so that events delivered by the inner event loop don't cause non-reentrant functions to be
     * called, which usually wreaks havoc.
     *
     * Note that the event loop started by this method does not process user input events, which means
     * your user interface will effectively be blocked. Other events like paint or network events are
     * still being processed. The advantage of not processing user input events is that the chance of
     * accidental reentrance is greatly reduced. Still you should avoid calling this function.
     *
     * Returns \c true if the job has been executed without error, \c false otherwise
     */
    bool exec();

    /*!
     * \value NoError Indicates there is no error
     * \value KilledJobError Indicates the job was killed
     * \value UserDefinedError Subclasses should define error codes starting at this value
     */
    enum {
        NoError = 0,
        KilledJobError = 1,
        UserDefinedError = 100,
    };

    /*!
     * Returns the error code, if there has been an error.
     *
     * Only call this method from the slot connected to result().
     *
     * Returns the error code for this job, 0 if no error.
     */
    int error() const;

    /*!
     * Returns the error text if there has been an error.
     *
     * Only call if error is not 0.
     *
     * This is usually some extra data associated with the error,
     * such as a URL.  Use errorString() to get a human-readable,
     * translated message.
     *
     * Returns a string to help understand the error
     */
    QString errorText() const;

    /*!
     * A human-readable error message.
     *
     * This provides a translated, human-readable description of the
     * error.  Only call if error is not 0.
     *
     * Subclasses should implement this to create a translated
     * error message from the error code and error text.
     * For example:
     * \code
     * if (error() == ReadFailed) {
     *   i18n("Could not read \"%1\"", errorText());
     * }
     * \endcode
     *
     * Returns a translated error message, providing error() is not 0
     */
    virtual QString errorString() const;

    /*!
     * Returns the processed amount of a given unit for this job.
     *
     * \a unit the unit of the requested amount
     */
    Q_SCRIPTABLE qulonglong processedAmount(Unit unit) const;

    /*!
     * Returns the total amount of a given unit for this job.
     *
     * \a unit the unit of the requested amount
     */
    Q_SCRIPTABLE qulonglong totalAmount(Unit unit) const;

    /*!
     * Returns the overall progress of this job
     */
    unsigned long percent() const;

    /*!
     * Sets the auto-delete property of the job. If \a autodelete is
     * set to \c false the job will not delete itself once it is finished.
     *
     * The default for any KJob is to automatically delete itself, which
     * implies that the job was created on the heap (using new).
     * If the job is created on the stack (which isn't the typical use-case
     * for a job) then you must set auto-delete to \c false, otherwise you
     * could get a crash when the job finishes and tries to delete itself.
     *
     * \note If you set auto-delete to \c false then you need to kill the
     * job manually, ideally by calling kill().
     *
     * \a autodelete set to \c false to disable automatic deletion
     * of the job.
     */
    void setAutoDelete(bool autodelete);

    /*!
     * Returns whether this job automatically deletes itself once
     * the job is finished.
     */
    bool isAutoDelete() const;

    /*!
     * This method can be used to indicate to classes listening to signals from a job
     * that they should ideally show a progress bar, but not a finished notification.
     *
     * For example when opening a remote URL, a job will emit the progress of the
     * download, which can be used to show a progress dialog or a Plasma notification,
     * then when the job is done it'll emit e.g. the finished signal. Showing the user the
     * progress dialog is useful, however the dialog/notification about the download being
     * finished isn't of much interest, because the user can see the application that invoked
     * the job opening the actual file that was downloaded.
     *
     * \since 5.92
     */
    void setFinishedNotificationHidden(bool hide = true);

    /*!
     * Whether to not show a finished notification when a job's finished
     * signal is emitted.
     *
     * \sa setFinishedNotificationHidden()
     *
     * \since 5.92
     */
    bool isFinishedNotificationHidden() const;

    /*!
     * Returns \c true if this job was started with exec(), which starts a nested event-loop
     * (with QEventLoop::ExcludeUserInputEvents, which blocks the GUI), otherwise returns
     * \c false which indicates this job was started asynchronously with start().
     *
     * This is useful for code that for example shows a dialog to ask the user a question,
     * and that would be no-op since the user cannot interact with the dialog.
     *
     * \since 5.95
     */
    bool isStartedWithExec() const;

    /*!
     * The number of milliseconds the job has been running for.
     * Starting from the last start() call.
     *
     * Sub-classes must call startElapsedTimer() from their start() implementation, to get elapsedTime() measurement. Otherwise this method will always return
     * 0.
     *
     * The time when paused is excluded.
     *
     * \since 6.8
     */
    qint64 elapsedTime() const;

Q_SIGNALS:

    /*!
     * Emitted when the job is finished, in any case. It is used to notify
     * observers that the job is terminated and that progress can be hidden.
     *
     * Since 5.75 this signal is guaranteed to be emitted exactly once.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use emitResult() instead.
     *
     * In general, to be notified of a job's completion, client code should connect to result()
     * rather than finished(), so that kill(Quietly) is indeed quiet.
     * However if you store a list of jobs and they might get killed silently,
     * then you must connect to this instead of result(), to avoid dangling pointers in your list.
     *
     * \a job the job that emitted this signal
     *
     * \sa result
     */
    void finished(KJob *job, QPrivateSignal);

    /*!
     * Emitted when the job is suspended.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob.
     *
     * \a job the job that emitted this signal
     */
    void suspended(KJob *job, QPrivateSignal);

    /*!
     * Emitted when the job is resumed.
     *
     * \a job the job that emitted this signal
     */
    void resumed(KJob *job, QPrivateSignal);

    /*!
     * Emitted when the job is finished (except when killed with KJob::Quietly).
     *
     * Since 5.75 this signal is guaranteed to be emitted at most once.
     *
     * Use error to know if the job was finished with error.
     *
     * This is a private signal, it can't be emitted directly by subclasses of
     * KJob, use emitResult() instead.
     *
     * Please connect to this signal instead of finished.
     *
     * \a job the job that emitted this signal
     *
     * \sa kill
     */
    void result(KJob *job, QPrivateSignal);

    /*!
     * Emitted to display general description of this job. A description has
     * a title and two optional fields which can be used to complete the
     * description.
     *
     * Examples of titles are "Copying", "Creating resource", etc.
     * The fields of the description can be "Source" with an URL, and,
     * "Destination" with an URL for a "Copying" description.
     *
     * \a job the job that emitted this signal
     *
     * \a title the general description of the job
     *
     * \a field1 first field (localized name and value)
     *
     * \a field2 second field (localized name and value)
     */
    void description(KJob *job,
                     const QString &title,
                     const QPair<QString, QString> &field1 = QPair<QString, QString>(),
                     const QPair<QString, QString> &field2 = QPair<QString, QString>());
    /*!
     * Emitted to display state information about this job.
     * Examples of message are "Resolving host", "Connecting to host...", etc.
     *
     * \a job the job that emitted this signal
     *
     * \a message the info message
     */
    void infoMessage(KJob *job, const QString &message);

    /*!
     * Emitted to display a warning about this job.
     *
     * \a job the job that emitted this signal
     *
     * \a message the warning message
     */
    void warning(KJob *job, const QString &message);

Q_SIGNALS:
    /*!
     * Emitted when we know the amount the job will have to process. The unit of this
     * amount is sent too. It can be emitted several times if the job manages several
     * different units.
     *
     * \note This is a private signal, it shouldn't be emitted directly by subclasses of
     * KJob, use setTotalAmount() instead.
     *
     * \a job the job that emitted this signal
     *
     * \a unit the unit of the total amount
     *
     * \a amount the total amount
     *
     * \since 5.80
     */
    void totalAmountChanged(KJob *job, KJob::Unit unit, qulonglong amount, QPrivateSignal);

    /*!
     * Regularly emitted to show the progress of this job by giving the current amount.
     * The unit of this amount is sent too. It can be emitted several times if the job
     * manages several different units.
     *
     * \note This is a private signal, it shouldn't be emitted directly by subclasses of
     * KJob, use setProcessedAmount() instead.
     *
     * \a job the job that emitted this signal
     *
     * \a unit the unit of the processed amount
     *
     * \a amount the processed amount
     *
     * \since 5.80
     */
    void processedAmountChanged(KJob *job, KJob::Unit unit, qulonglong amount, QPrivateSignal);

    /*!
     * Emitted when we know the size of this job (data size in bytes for transfers,
     * number of entries for listings, etc).
     *
     * \note This is a private signal, it shouldn't be emitted directly by subclasses of
     * KJob, use setTotalAmount() instead.
     *
     * \a job the job that emitted this signal
     *
     * \a size the total size
     */
    void totalSize(KJob *job, qulonglong size);

    /*!
     * Regularly emitted to show the progress of this job
     * (current data size in bytes for transfers, entries listed, etc.).
     *
     * \note This is a private signal, it shouldn't be emitted directly by subclasses of
     * KJob, use setProcessedAmount() instead.
     *
     * \a job the job that emitted this signal
     *
     * \a size the processed size
     */
    void processedSize(KJob *job, qulonglong size);

    /*!
     * Progress signal showing the overall progress of the job. This is
     * valid for any kind of job, and allows using a progress bar very
     * easily. (see KProgressBar).
     *
     * Note that this signal is not emitted for finished jobs.
     *
     * \note This is a private signal, it shouldn't be emitted directly
     * by subclasses of KJob, use emitPercent(), setPercent() setTotalAmount()
     * or setProcessedAmount() instead.
     *
     * \a job the job that emitted this signal
     *
     * \a percent the percentage
     *
     * \since 5.80
     */
    void percentChanged(KJob *job, unsigned long percent, QPrivateSignal);

    /*!
     * Emitted to display information about the speed of this job.
     *
     * \note This is a private signal, it shouldn't be emitted directly by subclasses of
     * KJob, use emitSpeed() instead.
     *
     * \a job the job that emitted this signal
     *
     * \a speed the speed in bytes/s
     */
    void speed(KJob *job, unsigned long speed);

protected:
    /*!
     * Returns if the job has been finished and has emitted the finished() signal.
     * \sa finished()
     * \since 5.75
     */
    // KF6 TODO: make public. Useful at least for unittests that run multiple jobs in parallel.
    bool isFinished() const;

    /*!
     * Sets the error code.
     *
     * It should be called when an error
     * is encountered in the job, just before calling emitResult().
     *
     * You should define an (anonymous) enum of error codes,
     * with values starting at KJob::UserDefinedError, and use
     * those.  For example,
     * \code
     * enum {
     *   InvalidFoo = UserDefinedError,
     *   BarNotFound,
     * };
     * \endcode
     *
     * \a errorCode the error code
     *
     * \sa emitResult()
     */
    void setError(int errorCode);

    /*!
     * Sets the error text.
     *
     * It should be called when an error
     * is encountered in the job, just before calling emitResult().
     *
     * Provides extra information about the error that cannot be
     * determined directly from the error code.  For example, a
     * URL or filename.  This string is not normally translatable.
     *
     * \a errorText the error text
     *
     * \sa emitResult(), errorString(), setError()
     */
    void setErrorText(const QString &errorText);

    /*!
     * Sets the processed size. The processedAmount() and percent() signals
     * are emitted if the values changed. The percent() signal is emitted
     * only for the progress unit.
     *
     * \a unit the unit of the new processed amount
     *
     * \a amount the new processed amount
     */
    void setProcessedAmount(Unit unit, qulonglong amount);

    /*!
     * Sets the total size. The totalSize() and percent() signals
     * are emitted if the values changed. The percent() signal is emitted
     * only for the progress unit.
     *
     * \a unit the unit of the new total amount
     *
     * \a amount the new total amount
     */
    void setTotalAmount(Unit unit, qulonglong amount);

    /*!
     * Sets the unit that will be used internally to calculate
     * the progress percentage.
     * The default progress unit is Bytes.
     * \since 5.76
     */
    void setProgressUnit(Unit unit);

    /*!
     * Sets the overall progress of the job. The percent() signal
     * is emitted if the value changed.
     *
     * The job takes care of this if you call setProcessedAmount
     * in Bytes (or the unit set by setProgressUnit).
     * This method allows you to set your own progress, as an alternative.
     *
     * \a percentage the new overall progress
     */
    void setPercent(unsigned long percentage);

    /*!
     * Utility function to emit the result signal, and end this job.
     * It first notifies the observers to hide the progress for this job using
     * the finished() signal.
     *
     * \note Deletes this job using deleteLater().
     *
     * \sa result()
     * \sa finished()
     */
    void emitResult();

    /*!
     * Utility function for inherited jobs.
     * Emits the percent signal if bigger than previous value,
     * after calculating it from the parameters.
     *
     * \a processedAmount the processed amount
     *
     * \a totalAmount the total amount
     *
     * \sa percent()
     */
    void emitPercent(qulonglong processedAmount, qulonglong totalAmount);

    /*!
     * Utility function for inherited jobs.
     * Emits the speed signal and starts the timer for removing that info
     *
     * \a speed the speed in bytes/s
     */
    void emitSpeed(unsigned long speed);

    /*!
     * Starts the internal elapsed time measurement timer.
     *
     * Sub-classes must call startElapsedTimer() from their start() implementation, to get elapsedTime() measurement.
     * Otherwise elapsedTimer() method will always return 0.
     *
     * \since 6.8
     */
    void startElapsedTimer();

protected:
    std::unique_ptr<KJobPrivate> const d_ptr;

    KCOREADDONS_NO_EXPORT KJob(KJobPrivate &dd, QObject *parent);

private:
    KCOREADDONS_NO_EXPORT void finishJob(bool emitResult);

    Q_DECLARE_PRIVATE(KJob)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KJob::Capabilities)

#endif
