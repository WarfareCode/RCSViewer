#include "OrientationCallback.h"
#include <osg/AnimationPath>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Switch>
#include <osg/PositionAttitudeTransform>

OrientationCallback::OrientationCallback()
{
	m_dLastTime = 0.0;
	m_dLastTime2 = 0.0;
	m_dTimeInterval = 1.0;
	m_pArray = new osg::Vec3Array();

	m_nState = 0;
}

OrientationCallback::~OrientationCallback()
{

}

void OrientationCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
	{
		double dTime = nv->getFrameStamp()->getSimulationTime();
		static double s_dStartTime = dTime;

		osg::Node* pTargetNode = DataManager::Instance()->GetTargetObjectNode();
		osg::Node* pPlaneNode = DataManager::Instance()->GetAerocraftNode();

		osg::Vec3 planePos = pPlaneNode->getBound().center() * osg::computeLocalToWorld(pPlaneNode->getParentalNodePaths()[0]);
		osg::Vec3 targetPos = pTargetNode->getBound().center() * osg::computeLocalToWorld(pTargetNode->getParentalNodePaths()[0]);

		//每转过一圈后，对轨迹线进行清除
		osg::PositionAttitudeTransform* pMatrixTrans = DataManager::Instance()->GetPlaneParentNode();
		if (pMatrixTrans)
		{
			osg::Callback* pCallBack = pMatrixTrans->getUpdateCallback();
			osg::AnimationPathCallback* pAnimationCallback = dynamic_cast<osg::AnimationPathCallback*>(pCallBack);

			if (pAnimationCallback)
			{
				osg::AnimationPath* pAnimationPath = pAnimationCallback->getAnimationPath();
				double dPeriod = pAnimationPath->getPeriod();
				static int s_nLoopCount = (dTime - pAnimationPath->getFirstTime()) / dPeriod;
				int nLoopCount = (dTime - pAnimationPath->getFirstTime()) / dPeriod;

				//转完一圈，清除飞行轨迹，清除plot内容
				if (s_nLoopCount != nLoopCount)
				{
					s_nLoopCount = nLoopCount;

					//清除轨迹线
					m_pArray->clear();

					//清除plot
				}
			}
		}

		if ((dTime - m_dLastTime) > 0.1)
		{
			m_dLastTime = dTime;

			m_pArray->push_back(planePos);

			osg::Geode* pPlanePath = DataManager::Instance()->GetPlanePath();
			if (pPlanePath == nullptr)
				return;

			osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry();
			{
				geometry->setVertexArray(m_pArray.get());

				osg::Vec4Array* colors = new osg::Vec4Array;
				colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
				geometry->setColorArray(colors, osg::Array::BIND_OVERALL);

				geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, m_pArray->size()));
			}

			osg::Drawable* pDrawable = pPlanePath->getDrawable(0);
			pPlanePath->removeDrawable(pDrawable);
			pPlanePath->addDrawable(geometry.get());
		}
		else if ((dTime - m_dLastTime2) > 0.025)
		{
			m_dLastTime2 = dTime;

			osg::Matrix matrix;
			osg::MatrixTransform* pTransform = dynamic_cast<osg::MatrixTransform*>(node);
			matrix.makeTranslate(planePos);
			pTransform->setMatrix(matrix);

			osg::Switch* pSwitch = dynamic_cast<osg::Switch*>(pTransform->getChild(0));

			osg::Vec3 vec0(0, -1, 0);
			osg::Vec3 vec1 = targetPos - planePos;

			/*static*/ double dDistance = vec1.length();
			double dSpeed = 0.05;

			bool bTag = true;

			//根据m_nState来修改s_dStartTime。否则会有问题。
			if (m_nState == 0)
			{
				m_dInterValTemp = dTime;
				m_nState = 1;
				s_dStartTime = dTime;

				pSwitch->setAllChildrenOff();
			}

			if (m_nState == 1 && (dTime - m_dInterValTemp) > m_dTimeInterval)
			{
				m_nState = 2;
				s_dStartTime = dTime;

				pSwitch->setAllChildrenOn();
			}

			double dSize0 = 2.0;
			if (m_nState > 1)
			{

				int nChildNum = pSwitch->getNumChildren();
				for (int i = 0; i < nChildNum; i++)
				{
					double dSize = (dTime - s_dStartTime - 0.02 * i)* dSpeed / (dDistance * 2.0);
					dSize = dSize - (int)dSize;
					if (i == nChildNum - 1)
					{
						dSize0 = dSize;
					}

					if (dSize >= 0.0)
					{
						pSwitch->setValue(i, true);
					}
					else
					{
						pSwitch->setValue(i, false);
					}

					if (m_nState == 3)
					{
						if (dSize < 0.4)
						{
							pSwitch->setValue(i, false);
						}
					}

					double dOffset = 0.0;
					if (dSize < 0.5)
					{
						dOffset = dDistance * 2.0 * dSize;
					}
					else
					{
						dOffset = (1.0 - dSize) * dDistance * 2.0;
					}

					osg::Matrix masc;
					masc.makeScale(dSize, dSize, dSize);

					osg::Matrix matr;
					matr.makeTranslate(0.0, -dOffset, 0.0);
					masc.postMult(matr);

					osg::Matrix maro;
					maro.makeRotate(vec0, vec1);
					masc.postMult(maro);

					osg::MatrixTransform* pSubTransform = dynamic_cast<osg::MatrixTransform*>(pSwitch->getChild(i));
					pSubTransform->setMatrix(masc);
				}
			}

			if (dSize0 > 0.5 /*&& dSize0 < 0.55 */&& m_nState == 2)
			{
				m_nState = 3;
				//pSwitch->setAllChildrenOff();
			}

			if (dSize0 > 0.9/* && dSize0 < 0.1 && dSize0 > m_dLastSize*/ && m_nState == 3)
			{
				pSwitch->setAllChildrenOff();
				m_nState = 0;
			}

			m_dLastSize = dSize0;
		}
	}

	traverse(node, nv);
}