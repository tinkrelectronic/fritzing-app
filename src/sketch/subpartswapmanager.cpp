/*********************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2024 Fritzing

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************/

#include "subpartswapmanager.h"

#include "../referencemodel/referencemodel.h"
#include "../items/itembase.h"
#include "../utils/textutils.h"
#include "../utils/graphicsutils.h"
#include "../viewgeometry.h"
#include "../debugdialog.h"

SubpartSwapManager::SubpartSwapManager(ReferenceModel *referenceModel)
	: m_referenceModel(referenceModel) {
}

//-------------------------------------------------------------------------------------------
// View independent functions to be used once per swap session
void SubpartSwapManager::generateSubpartModelIndices(const NewMainModuleID &newModuleID) {
	ModelPart * newModelPart = m_referenceModel->retrieveModelPart(newModuleID);
	if (!newModelPart->hasSubparts()) return;
	ModelPartShared * modelPartShared = newModelPart->modelPartShared();
	if (!modelPartShared) return;
	for (ModelPartShared* mps : modelPartShared->subparts()) {
		long newSubModelIndex = ModelPart::nextIndex();
		m_subPartNewModuleID2NewModelIndexMap.insert(mps->moduleID(), newSubModelIndex);
		long newSubID = ItemBase::getNextID(newSubModelIndex);
		m_subPartNewModuleID2NewSubIDMap.insert(mps->moduleID(), newSubID);
	}
}

void SubpartSwapManager::correlateOldAndNewSubparts(const NewMainModuleID &newModuleID, ItemBase *itemBase) {
	if (!itemBase) return;
	ModelPart * newModelPart = m_referenceModel->retrieveModelPart(newModuleID);
	if (!newModelPart->hasSubparts()) return;
	ModelPartShared * modelPartShared = newModelPart->modelPartShared();
	if (!modelPartShared) return;
	if (itemBase->subparts().count() != modelPartShared->subparts().count()) {
		DebugDialog::debug(QString("SketchWidget::swapStart: subpart counts for old and new item disagree: old count: %1 new count: %2").arg(itemBase->subparts().count()).arg(modelPartShared->subparts().count()));
	}
	QMap<QString, ItemBase*> subpartMap;

	QStringList oldModuleIDs, newModuleIDs;
	for (ItemBase* subpart : itemBase->subparts()) {
		oldModuleIDs << subpart->moduleID();
	}
	for (ModelPartShared* mps : modelPartShared->subparts()) {
		newModuleIDs << mps->moduleID();
	}
	QString oldPrefix = TextUtils::commonPrefix(oldModuleIDs);
	QString oldSuffix = TextUtils::commonSuffix(oldModuleIDs);
	QString newPrefix = TextUtils::commonPrefix(newModuleIDs);
	QString newSuffix = TextUtils::commonSuffix(newModuleIDs);

	for (ItemBase* subpart : itemBase->subparts()) {
		QString id = subpart->moduleID();
		QString uniqueSubString = id.mid(oldPrefix.length(), id.length() - oldPrefix.length() - oldSuffix.length());
		subpartMap.insert(uniqueSubString, subpart);
	}

	for (ModelPartShared* mps : modelPartShared->subparts()) {
		QString newSubModuleID = mps->moduleID();
		QString uniqueSubString = newSubModuleID.mid(newPrefix.length(), newSubModuleID.length() - newPrefix.length() - newSuffix.length());
		ItemBase * subPart = nullptr;
		if (subpartMap.contains(uniqueSubString)) {
			subPart = subpartMap[uniqueSubString];
			m_subPartModuleIDNew2OldMap.insert(mps->moduleID(), subPart->moduleID());
			m_subPartModuleIDOld2NewMap.insert(subPart->moduleID(), mps->moduleID());
		} else {
			DebugDialog::debug(QString("SketchWidget::swapStart old subpart with moduleID: %1 not found").arg(mps->moduleID()));
		}
	}
}

//-------------------------------------------------------------------------------------------

void SubpartSwapManager::resetOldSubParts(ItemBase * itemBase) {
	m_subPartMap.clear();
	for (ItemBase* subPart : itemBase->subparts()) {
		m_subPartMap.insert(subPart->moduleID(), subPart);
	}
}

ItemBase * SubpartSwapManager::extractSubPart(const NewSubModuleID & newModuleID) {
	QString oldSubPartModuleID = getOldModuleID(newModuleID);
	ItemBase * subPart = m_subPartMap.value(oldSubPartModuleID, nullptr);
	m_subPartMap.remove(oldSubPartModuleID);
	return subPart;
}

bool SubpartSwapManager::newModuleIDWasCorrelated(const NewSubModuleID & newModuleID) const {
	return m_subPartModuleIDNew2OldMap.contains(newModuleID);
}

NewModelIndex SubpartSwapManager::getNewModelIndex(const NewSubModuleID &newModuleID) const {
	return m_subPartNewModuleID2NewModelIndexMap.value(newModuleID, -1);
}

QList<NewSubModuleID> SubpartSwapManager::getNewModuleIDs() const {
	return m_subPartNewModuleID2NewModelIndexMap.keys();
}

NewSubID SubpartSwapManager::getNewSubID(const NewSubModuleID &newModuleID) const {
	return m_subPartNewModuleID2NewSubIDMap.value(newModuleID, -1);
}

OldSubModuleID SubpartSwapManager::getOldModuleID(const NewSubModuleID &newModuleID) const {
	return m_subPartModuleIDNew2OldMap.value(newModuleID);
}

NewSubModuleID SubpartSwapManager::getNewModuleID(const OldSubModuleID &oldModuleID) const {
	return m_subPartModuleIDOld2NewMap.value(oldModuleID);
}
