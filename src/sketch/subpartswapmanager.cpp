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

#include "referencemodel/referencemodel.h"
#include "items/itembase.h"

SubpartSwapManager::SubpartSwapManager(ReferenceModel *referenceModel)
	: m_referenceModel(referenceModel) {
}

//-------------------------------------------------------------------------------------------
// View independent functions to be used once per swap session
void SubpartSwapManager::generateSubpartModelIndices(const NewMainModuleID &newModuleID) {
	for (ModelPartShared* mps : subparts(newModuleID)) {
		long newSubModelIndex = ModelPart::nextIndex();
		m_subPartNewModuleID2NewModelIndexMap.insert(mps->moduleID(), newSubModelIndex);
		long newSubID = ItemBase::getNextID(newSubModelIndex);
		m_subPartNewModuleID2NewSubIDMap.insert(mps->moduleID(), newSubID);
	}
}

void SubpartSwapManager::correlateOldAndNewSubparts(const NewMainModuleID &newModuleID, ItemBase *itemBase) {
	auto subpartList = subparts(newModuleID);
	if (subpartList.count() <= 0) return;
	QMap<QString, ItemBase*> subpartMap;

	for (ItemBase* subpart : itemBase->subparts())
		subpartMap.insert(subpart->subpartID(), subpart);

	for (ModelPartShared* mps : subpartList)
		if (subpartMap.contains(mps->subpartID()))
			m_subPartModuleIDNew2OldMap.insert(mps->moduleID(), subpartMap[mps->subpartID()]->moduleID());
}

//-------------------------------------------------------------------------------------------

const QList< QPointer<ModelPartShared> > & SubpartSwapManager::subparts(const NewMainModuleID &newModuleID) {
	static const QList< QPointer<ModelPartShared> > emptyList;
	ModelPart * newModelPart = m_referenceModel->retrieveModelPart(newModuleID);
	if (!newModelPart) return emptyList;
	if (!newModelPart->hasSubparts()) return emptyList;
	ModelPartShared * modelPartShared = newModelPart->modelPartShared();
	if (!modelPartShared) return emptyList;
	return modelPartShared->subparts();
}

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
