/*
 * Copyright (C) 2013 Jolla Mobile <andrew.den.exter@jollamobile.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of Nemo Mobile nor the names of its contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include <QObject>
#include <QtTest>

#include <QContactName>
#ifdef USING_QTPIM
#include <QContactManager>
#endif

#include "seasidefilteredmodel.h"
#include "seasidecache.h"
#include "seasideperson.h"

Q_DECLARE_METATYPE(QModelIndex)

class tst_SeasideFilteredModel : public QObject
{
    Q_OBJECT
public:
    typedef SeasideFilteredModel::ContactIdType ContactIdType;

    tst_SeasideFilteredModel();

private slots:
    void init();
    void populated();
    void filterType();
    void filterPattern();
    void filterEmail();
    void rowsInserted();
    void rowsRemoved();
    void dataChanged();
    void data();
    void filterId();
    void searchByFirstNameCharacter();
    void lookupById();
    void requiredProperty();
    void mixedFilters();

private:
    QVariant idAt(int index) const { return QVariant::fromValue<ContactIdType>(cache.idAt(index)); }

    SeasideCache cache;
};

tst_SeasideFilteredModel::tst_SeasideFilteredModel()
{
    qRegisterMetaType<QModelIndex>();
}


void tst_SeasideFilteredModel::init()
{
#ifdef USING_QTPIM
    // The backend must be loaded for cache reset to work correctly
    QContactManager cm("org.nemomobile.contacts.sqlite");
#endif

    cache.reset();
}

void tst_SeasideFilteredModel::populated()
{
    SeasideFilteredModel model;

    QCOMPARE(model.isPopulated(), false);

    QSignalSpy spy(&model, SIGNAL(populatedChanged()));

    model.setFilterType(SeasideFilteredModel::FilterNone);
    QCOMPARE(model.isPopulated(), false);
    QCOMPARE(spy.count(), 0);

    cache.populate(SeasideCache::FilterFavorites);

    QCOMPARE(spy.count(), 0);
    model.setFilterType(SeasideFilteredModel::FilterFavorites);
    QCOMPARE(model.isPopulated(), true);
    QCOMPARE(spy.count(), 1);

    model.setFilterType(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.isPopulated(), false);
    QCOMPARE(spy.count(), 2);

    cache.populate(SeasideCache::FilterAll);
    QCOMPARE(model.isPopulated(), true);
    QCOMPARE(spy.count(), 3);
}

void tst_SeasideFilteredModel::filterType()
{
    SeasideFilteredModel model;
    QSignalSpy typeSpy(&model, SIGNAL(filterTypeChanged()));
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    // 0 1 2 3 4 5 6
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(typeSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);

    // 0 1 2 3 4 5 6
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(typeSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);

    // 2 5 6
    model.setFilterType(SeasideFilteredModel::FilterFavorites);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterFavorites);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(removedSpy.at(1).at(1).value<int>(), 1);
    QCOMPARE(removedSpy.at(1).at(2).value<int>(), 2);

    typeSpy.clear();
    removedSpy.clear();

    // 0 1 2 3 4 5 6
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 2);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(insertedSpy.at(1).at(1).value<int>(), 3);
    QCOMPARE(insertedSpy.at(1).at(2).value<int>(), 4);
    QCOMPARE(removedSpy.count(), 0);

    typeSpy.clear();
    insertedSpy.clear();

    //
    model.setFilterType(SeasideFilteredModel::FilterNone);
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterNone);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 6);
}

void tst_SeasideFilteredModel::filterPattern()
{
    SeasideFilteredModel model;
    QSignalSpy patternSpy(&model, SIGNAL(filterPatternChanged()));
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.filterPattern(), QString());
    QCOMPARE(model.rowCount(), 7);

    // 0 1 2 3 4
    model.setFilterPattern("a");
    QCOMPARE(model.filterPattern(), QString("a"));
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 5);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 6);

    patternSpy.clear();
    removedSpy.clear();

    // 0 1 2 4
    model.setFilterPattern("aA");
    QCOMPARE(model.filterPattern(), QString("aA"));
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 3);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 3);

    patternSpy.clear();
    removedSpy.clear();

    // 0 4
    model.setFilterPattern("aaronso");
    QCOMPARE(model.filterPattern(), QString("aaronso"));
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 1);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 2);

    patternSpy.clear();
    removedSpy.clear();

    // empty
    model.setFilterType(SeasideFilteredModel::FilterFavorites);
    QCOMPARE(model.filterPattern(), QString("aaronso"));
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(patternSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);

    removedSpy.clear();

    // 2
    model.setFilterPattern("Aa");
    QCOMPARE(model.filterPattern(), QString("Aa"));
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 0);
    QCOMPARE(removedSpy.count(), 0);

    patternSpy.clear();
    insertedSpy.clear();

    // 0 1 2 4
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QCOMPARE(model.filterPattern(), QString("Aa"));
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(patternSpy.count(), 0);
    QCOMPARE(insertedSpy.count(), 2);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(insertedSpy.at(1).at(1).value<int>(), 3);
    QCOMPARE(insertedSpy.at(1).at(2).value<int>(), 3);
    QCOMPARE(removedSpy.count(), 0);

    insertedSpy.clear();

    // 0 1 2 3 4
    model.setFilterPattern("a");
    QCOMPARE(model.filterPattern(), QString("a"));
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 3);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 3);
    QCOMPARE(removedSpy.count(), 0);

    patternSpy.clear();
    insertedSpy.clear();

    // 2 3 5
    model.setFilterPattern("Jo");
    QCOMPARE(model.filterPattern(), QString("Jo"));
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 2);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(removedSpy.at(1).at(1).value<int>(), 2);
    QCOMPARE(removedSpy.at(1).at(2).value<int>(), 2);

    patternSpy.clear();
    insertedSpy.clear();
    removedSpy.clear();

    // 0 1 2 3 4 5 6
    model.setFilterPattern(QString());
    QCOMPARE(model.filterPattern(), QString());
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 3);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(insertedSpy.at(1).at(1).value<int>(), 4);
    QCOMPARE(insertedSpy.at(1).at(2).value<int>(), 4);
    QCOMPARE(insertedSpy.at(2).at(1).value<int>(), 6);
    QCOMPARE(insertedSpy.at(2).at(2).value<int>(), 6);
    QCOMPARE(removedSpy.count(), 0);

    patternSpy.clear();
    insertedSpy.clear();

    // 2 3 4 5
    model.setFilterType(SeasideFilteredModel::FilterNone);
    model.setFilterPattern("J");
    QCOMPARE(model.rowCount(), 4);

    patternSpy.clear();
    insertedSpy.clear();
    removedSpy.clear();

    // empty
    model.setFilterPattern(QString());
    QCOMPARE(model.filterPattern(), QString());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(patternSpy.count(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 3);

    patternSpy.clear();
    removedSpy.clear();
}

void tst_SeasideFilteredModel::filterEmail()
{
    SeasideFilteredModel model;
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.filterPattern(), QString());
    QCOMPARE(model.rowCount(), 7);

    // 0 1 2 3 4 5
    model.setFilterPattern("example");
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 6);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 6);

    removedSpy.clear();

    // 0 1 2 3 4 5
    model.setFilterPattern("example.");
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);

    // 0 1 2
    model.setFilterPattern("example.com");
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 3);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 5);

    removedSpy.clear();

    // 3 4 5
    model.setFilterPattern("example.org");
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 2);

    insertedSpy.clear();
    removedSpy.clear();

    // 4 5
    model.setFilterPattern("examplez.org");
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 0);

    removedSpy.clear();

    // 4
    model.setFilterPattern("jay@examplez.org");
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 1);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);

    removedSpy.clear();

    // 4
    model.setFilterPattern("jay");
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);
}

void tst_SeasideFilteredModel::rowsInserted()
{
    // Remove the exitsting index values
    cache.remove(SeasideCache::FilterAll, 0, 7);

    cache.insert(SeasideCache::FilterAll, 0, QVector<ContactIdType>()
            << cache.idAt(2) << cache.idAt(5));

    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));

    // 2 5
    QCOMPARE(model.rowCount(), 2);

    // 0 1 2 5
    cache.insert(SeasideCache::FilterAll, 0, QVector<ContactIdType>()
            << cache.idAt(0) << cache.idAt(1));
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);

    insertedSpy.clear();

    // 1
    model.setFilterPattern("Ar");
    QCOMPARE(model.rowCount(), 1);

    // 1 3
    cache.insert(SeasideCache::FilterAll, 3, QVector<ContactIdType>()
            << cache.idAt(3) << cache.idAt(4));
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 1);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 1);
}

void tst_SeasideFilteredModel::rowsRemoved()
{
    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    // 0 1 2 3 4 5 6
    QCOMPARE(model.rowCount(), 7);

    // 2 3 4 5 6
    cache.remove(SeasideCache::FilterAll, 0, 2);
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);

    // 2 3 5
    model.setFilterPattern("Jo");
    QCOMPARE(model.rowCount(), 3);

    removedSpy.clear();

    // 2 5
    cache.remove(SeasideCache::FilterAll, 1, 2);
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 1);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);
}

void tst_SeasideFilteredModel::dataChanged()
{
    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);

    // 0 1 2 3 4 5 6
    QCOMPARE(model.rowCount(), 7);

    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));
    QSignalSpy changedSpy(&model, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    // 0 1 2 3 4 5 6
    cache.setFirstName(SeasideCache::FilterAll, 2, "Doug");
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(changedSpy.at(0).at(0).value<QModelIndex>(), model.index(QModelIndex(), 2, 0));
    QCOMPARE(changedSpy.at(0).at(1).value<QModelIndex>(), model.index(QModelIndex(), 2, 0));

    // 0 1 3 4
    model.setFilterPattern("A");
    QCOMPARE(model.rowCount(), 4);

    removedSpy.clear();
    changedSpy.clear();

    cache.setFirstName(SeasideCache::FilterAll, 2, "Aaron");
    QCOMPARE(model.rowCount(), 5);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 2);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(changedSpy.count(), 0);

    insertedSpy.clear();

    // 0 1 2 3 4 5
    cache.setFirstName(SeasideCache::FilterAll, 2, "Doug");
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 2);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(changedSpy.count(), 0);
}

void tst_SeasideFilteredModel::data()
{
    QModelIndex index;

    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);

    QCOMPARE(model.rowCount(), 7);

    index = model.index(QModelIndex(), 0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Aaron Aaronson"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Aaron"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Aaronson"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("A"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("image://theme/icon-m-telephony-contact-avatar")));

    index = model.index(QModelIndex(), 5, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Joe Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Joe"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("J"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("file:///cache/joe.jpg")));

    model.setFilterType(SeasideFilteredModel::FilterFavorites);

    index = model.index(QModelIndex(), 0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Aaron Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Aaron"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("A"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("image://theme/icon-m-telephony-contact-avatar")));

    index = model.index(QModelIndex(), 2, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Robin Burchell"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Robin"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Burchell"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("R"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("image://theme/icon-m-telephony-contact-avatar")));

    model.setFilterType(SeasideFilteredModel::FilterOnline);

    index = model.index(QModelIndex(), 0, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Aaron Arthur"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Aaron"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Arthur"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("A"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("image://theme/icon-m-telephony-contact-avatar")));

    index = model.index(QModelIndex(), 2, 0);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), QString("Joe Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::FirstNameRole).toString(), QString("Joe"));
    QCOMPARE(index.data(SeasideFilteredModel::LastNameRole).toString(), QString("Johns"));
    QCOMPARE(index.data(SeasideFilteredModel::SectionBucketRole).toString(), QString("J"));
    QCOMPARE(index.data(SeasideFilteredModel::AvatarRole).toUrl(), QUrl(QLatin1String("file:///cache/joe.jpg")));
}

void tst_SeasideFilteredModel::filterId()
{
    SeasideFilteredModel model;
    // 6: Robin Burchell

    // first name only
    model.setFilterPattern("R");            QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Ro");           QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Rob");          QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robi");         QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin");        QVERIFY(model.filterId(cache.idAt(6)));

    // match using last name
    model.setFilterPattern("B");            QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Bu");           QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Bur");          QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Burc");         QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Burch");        QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Burche");       QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Burchel");      QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Burchell");     QVERIFY(model.filterId(cache.idAt(6)));


    // first name plus last name
    model.setFilterPattern("Robin ");           QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin B");          QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin Bu");         QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin Bur");        QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin Burc");       QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin Burch");      QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin Burche");     QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin Burchel");    QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin Burchell");   QVERIFY(model.filterId(cache.idAt(6)));

    // match using funky spacing
    model.setFilterPattern("R B");  QVERIFY(model.filterId(cache.idAt(6)));
    model.setFilterPattern("R Bu"); QVERIFY(model.filterId(cache.idAt(6)));

    // test some that most definitely shouldn't match
    model.setFilterPattern("Robert");           QVERIFY(!model.filterId(cache.idAt(6)));
    model.setFilterPattern("Robin Brooks");     QVERIFY(!model.filterId(cache.idAt(6)));
    model.setFilterPattern("John Burchell");    QVERIFY(!model.filterId(cache.idAt(6)));
    model.setFilterPattern("Brooks");           QVERIFY(!model.filterId(cache.idAt(6)));
}

void tst_SeasideFilteredModel::searchByFirstNameCharacter()
{
    SeasideFilteredModel model;
    model.setSearchByFirstNameCharacter(true);

    model.setFilterPattern("R");
    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.rowCount(), 1);

    model.setFilterPattern("A");
    QCOMPARE(model.rowCount(), 4);

    model.setFilterPattern("r");    // not case-sensitive
    QCOMPARE(model.rowCount(), 1);
    model.setFilterPattern("aaron");    // only first letter counts
    QCOMPARE(model.rowCount(), 4);

    SeasideCache::CacheItem *cacheItem = SeasideCache::existingItem(cache.idAt(0));
    QString origName = cacheItem->contact.detail<QContactName>().firstName();
    cache.setFirstName(SeasideCache::FilterAll, 0, "123");
    model.setFilterPattern("#");    // non-letters
    QCOMPARE(model.rowCount(), 1);

    cache.setFirstName(SeasideCache::FilterAll, 0, origName);
    model.setFilterPattern("");
    QCOMPARE(model.rowCount(), 7);
    model.setFilterPattern("#");
    QCOMPARE(model.rowCount(), 0);
}

void tst_SeasideFilteredModel::lookupById()
{
    QModelIndex index;

    SeasideFilteredModel model;
    model.setFilterType(SeasideFilteredModel::FilterAll);

    QCOMPARE(model.rowCount(), 7);

    for (int i = 0; i < 7; ++i) {
        // Verify the lookup by integer
        QModelIndex index = model.index(QModelIndex(), i, 0);

        SeasidePerson *person = index.data(SeasideFilteredModel::PersonRole).value<SeasidePerson *>();
        QCOMPARE(model.personById(person->id()), person);
    }

    // Verify that invalid values yield nothing
    QCOMPARE(model.personById(0), static_cast<SeasidePerson *>(0));
    QCOMPARE(model.personById(666), static_cast<SeasidePerson *>(0));
}

void tst_SeasideFilteredModel::requiredProperty()
{
    SeasideFilteredModel model;
    QSignalSpy propertySpy(&model, SIGNAL(requiredPropertyChanged()));
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.requiredProperty(), int(SeasideFilteredModel::NoPropertyRequired));
    QCOMPARE(model.rowCount(), 7);

    // 0 3 4 6
    model.setRequiredProperty(SeasideFilteredModel::PhoneNumberRequired);
    QCOMPARE(model.requiredProperty(), int(SeasideFilteredModel::PhoneNumberRequired));
    QCOMPARE(propertySpy.count(), 1);
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 1);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(removedSpy.at(1).at(1).value<int>(), 3);
    QCOMPARE(removedSpy.at(1).at(2).value<int>(), 3);

    propertySpy.clear();
    removedSpy.clear();

    // 0 1 2 3 4 5
    model.setRequiredProperty(SeasideFilteredModel::EmailAddressRequired);
    QCOMPARE(model.requiredProperty(), int(SeasideFilteredModel::EmailAddressRequired));
    QCOMPARE(propertySpy.count(), 1);
    QCOMPARE(model.rowCount(), 6);
    QCOMPARE(insertedSpy.count(), 2);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 1);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(insertedSpy.at(1).at(1).value<int>(), 5);
    QCOMPARE(insertedSpy.at(1).at(2).value<int>(), 5);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 5);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 5);

    propertySpy.clear();
    insertedSpy.clear();
    removedSpy.clear();

    // 0 1 2 3 4 5 6
    model.setRequiredProperty(SeasideFilteredModel::EmailAddressRequired | SeasideFilteredModel::PhoneNumberRequired);
    QCOMPARE(model.requiredProperty(), SeasideFilteredModel::EmailAddressRequired  | SeasideFilteredModel::PhoneNumberRequired);
    QCOMPARE(propertySpy.count(), 1);
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 6);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 6);
    QCOMPARE(removedSpy.count(), 0);

    propertySpy.clear();
    insertedSpy.clear();
    removedSpy.clear();

    // No IM accounts in this data
    model.setRequiredProperty(SeasideFilteredModel::AccountUriRequired);
    QCOMPARE(model.requiredProperty(), int(SeasideFilteredModel::AccountUriRequired));
    QCOMPARE(propertySpy.count(), 1);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 6);

    propertySpy.clear();
    removedSpy.clear();

    model.setRequiredProperty(SeasideFilteredModel::NoPropertyRequired);
    QCOMPARE(model.requiredProperty(), int(SeasideFilteredModel::NoPropertyRequired));
    QCOMPARE(propertySpy.count(), 1);
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 6);
    QCOMPARE(removedSpy.count(), 0);
}

void tst_SeasideFilteredModel::mixedFilters()
{
    SeasideFilteredModel model;
    QSignalSpy insertedSpy(&model, SIGNAL(rowsInserted(QModelIndex,int,int)));
    QSignalSpy removedSpy(&model, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QCOMPARE(model.filterType(), SeasideFilteredModel::FilterAll);
    QCOMPARE(model.rowCount(), 7);

    // 0 3 4 6
    model.setRequiredProperty(SeasideFilteredModel::PhoneNumberRequired);
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 1);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 2);
    QCOMPARE(removedSpy.at(1).at(1).value<int>(), 3);
    QCOMPARE(removedSpy.at(1).at(2).value<int>(), 3);
    QCOMPARE(model.index(QModelIndex(), 0, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Aaronson"));
    QCOMPARE(model.index(QModelIndex(), 1, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Johns"));
    QCOMPARE(model.index(QModelIndex(), 2, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Aaronson"));
    QCOMPARE(model.index(QModelIndex(), 3, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Burchell"));

    removedSpy.clear();

    // 0 4
    model.setFilterPattern("Aaron");
    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 2);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 1);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);
    QCOMPARE(removedSpy.at(1).at(1).value<int>(), 2);
    QCOMPARE(removedSpy.at(1).at(1).value<int>(), 2);
    QCOMPARE(model.index(QModelIndex(), 0, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Aaronson"));
    QCOMPARE(model.index(QModelIndex(), 1, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Aaronson"));

    removedSpy.clear();

    // None
    model.setRequiredProperty(SeasideFilteredModel::AccountUriRequired);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(insertedSpy.count(), 0);
    QCOMPARE(removedSpy.count(), 1);
    QCOMPARE(removedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(removedSpy.at(0).at(2).value<int>(), 1);

    removedSpy.clear();

    // 0 1 2 4
    model.setRequiredProperty(SeasideFilteredModel::NoPropertyRequired);
    QCOMPARE(model.rowCount(), 4);
    QCOMPARE(insertedSpy.count(), 1);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 0);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 3);
    QCOMPARE(removedSpy.count(), 0);
    QCOMPARE(model.index(QModelIndex(), 0, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Aaronson"));
    QCOMPARE(model.index(QModelIndex(), 1, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Arthur"));
    QCOMPARE(model.index(QModelIndex(), 2, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Johns"));
    QCOMPARE(model.index(QModelIndex(), 3, 0).data(SeasideFilteredModel::LastNameRole).toString(), QString::fromLatin1("Aaronson"));

    insertedSpy.clear();

    // 0 1 2 3 4 5 6
    model.setFilterPattern(QString());
    QCOMPARE(model.rowCount(), 7);
    QCOMPARE(insertedSpy.count(), 2);
    QCOMPARE(insertedSpy.at(0).at(1).value<int>(), 3);
    QCOMPARE(insertedSpy.at(0).at(2).value<int>(), 3);
    QCOMPARE(insertedSpy.at(1).at(1).value<int>(), 5);
    QCOMPARE(insertedSpy.at(1).at(2).value<int>(), 6);
    QCOMPARE(removedSpy.count(), 0);
}

#include "tst_seasidefilteredmodel.moc"
QTEST_APPLESS_MAIN(tst_SeasideFilteredModel)
