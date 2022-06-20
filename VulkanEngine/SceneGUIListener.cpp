#include "VEInclude.h"

namespace ve {

	void SceneGUIListener::onDrawOverlay(veEvent event) {

		// Prepare context
		VESubrenderFW_Nuklear* pSubrender = (VESubrenderFW_Nuklear*)getRendererPointer()->getOverlay();
		if (pSubrender == nullptr) return;

		struct nk_context* ctx = pSubrender->getContext();

		// Draw test UI
        if (nk_begin(ctx, "Scene Info", nk_rect(1000, 50, 610, 800),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE)) {

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Scene objects:");

			// Draw scene object tree
			nk_layout_row_dynamic(ctx, 400, 1);
			if (nk_group_begin(ctx, "Scene objects", NULL)) {
				currtreenodeid = 0;
				addToTree(getSceneManagerPointer()->getRootSceneNode(), ctx);
				nk_group_end(ctx);
			}

			nk_layout_row_dynamic(ctx, 30, 1);

			// Select object info
			if (nodeSelected) {
				nk_labelf(ctx, NK_TEXT_LEFT, "Selected object:");
				nk_labelf(ctx, NK_TEXT_LEFT, selectedNode->getName().c_str());

				// Show type
				if (dynamic_cast<VELight*>(selectedNode) != nullptr) {
					nk_labelf(ctx, NK_TEXT_LEFT, "Type Light");
					addLightColourPicker(ctx, dynamic_cast<VELight*>(selectedNode));
				} else if (dynamic_cast<VECamera*>(selectedNode) != nullptr){
					nk_labelf(ctx, NK_TEXT_LEFT, "Type Camera");
				} else {
					nk_labelf(ctx, NK_TEXT_LEFT, "Type Entity");
				}

				// Position
				addPositionSliders(ctx, selectedNode);
			}

        }
		nk_end(ctx);
	}

	void SceneGUIListener::addToTree(VESceneNode* node, nk_context* ctx) {

		nk_layout_row_begin(ctx, NK_STATIC, 20, 3);
		{
			nk_layout_row_push(ctx, 35);

			if (selectedNode == node) {
				nk_labelf(ctx, NK_TEXT_CENTERED, "X");
			} else {
				if (nk_button_label(ctx, "O")) {
					selectedNode = node;
					nodeSelected = true;
				}
			}

			nk_layout_row_push(ctx, 515);
			nk_labelf(ctx, NK_TEXT_LEFT, node->getName().c_str());
		}
		nk_layout_row_end(ctx);

		std::vector<VESceneNode*> children = node->getChildrenList();

		for (int i = 0; i < children.size(); i++) {
			addToTree(children[i], ctx);
		}
	}

	// Position selection
	void SceneGUIListener::addPositionSliders(nk_context* ctx, VESceneNode* selectedNode) {

		glm::vec3 pos = selectedNode->getPosition();
		float xval = pos.x;
		float yval = pos.y;
		float zval = pos.z;

		// X
		nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
		{
			nk_layout_row_push(ctx, 60);
			nk_labelf(ctx, NK_TEXT_LEFT, "X: -50");

			nk_layout_row_push(ctx, 470);
			nk_slider_float(ctx, -50.0f, &xval, 50.0f, 0.5f);

			nk_layout_row_push(ctx, 20);
			nk_labelf(ctx, NK_TEXT_LEFT, "50");
		}
		nk_layout_row_end(ctx);

		// Y
		nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
		{
			nk_layout_row_push(ctx, 60);
			nk_labelf(ctx, NK_TEXT_LEFT, "Y: -50");

			nk_layout_row_push(ctx, 470);
			nk_slider_float(ctx, -50.0f, &yval, 50.0f, 0.5f);

			nk_layout_row_push(ctx, 20);
			nk_labelf(ctx, NK_TEXT_LEFT, "50");
		}
		nk_layout_row_end(ctx);

		// Z
		nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
		{
			nk_layout_row_push(ctx, 60);
			nk_labelf(ctx, NK_TEXT_LEFT, "Z: -50");

			nk_layout_row_push(ctx, 470);
			nk_slider_float(ctx, -50.0f, &zval, 50.0f, 0.5f);

			nk_layout_row_push(ctx, 20);
			nk_labelf(ctx, NK_TEXT_LEFT, "50");
		}
		nk_layout_row_end(ctx);

		selectedNode->setPosition(glm::vec3(xval, yval, zval));
	}

	// Colour selection
	void SceneGUIListener::addLightColourPicker(nk_context* ctx, VELight* node) {
		glm::vec4 v = node->m_col_diffuse;
		float ratios[] = { 0.08f, 0.92f };
		nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratios);
		nk_label(ctx, "R:", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &node->m_col_diffuse.r, 1, 0.01);
		nk_label(ctx, "G:", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &node->m_col_diffuse.g, 1, 0.01);
		nk_label(ctx, "B:", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &node->m_col_diffuse.b, 1, 0.01);
		nk_label(ctx, "A:", NK_TEXT_LEFT);
		nk_slider_float(ctx, 0, &node->m_col_diffuse.a, 1, 0.01);
	}
}