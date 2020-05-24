#include "VEInclude.h"

namespace ve {

	void SceneGUIListener::onDrawOverlay(veEvent event) {

		// Prepare context
		VESubrenderFW_Nuklear* pSubrender = (VESubrenderFW_Nuklear*)getRendererPointer()->getOverlay();
		if (pSubrender == nullptr) return;

		struct nk_context* ctx = pSubrender->getContext();

		// Draw test UI
        if (nk_begin(ctx, "Scene Info", nk_rect(1000, 50, 240, 400),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Scene objects:");

			// Draw scene object tree
			currtreenodeid = 0;
			addToTree(getSceneManagerPointer()->getRootSceneNode(), ctx);

			// Select object info
			if (nodeSelected) {
				nk_labelf(ctx, NK_TEXT_LEFT, "Selected object:");
				nk_labelf(ctx, NK_TEXT_LEFT, selectedNode->getName().c_str());
			}
        }
        nk_end(ctx);		
	}

	void SceneGUIListener::addToTree(VESceneNode* node, nk_context* ctx) {
		if (nk_tree_push_id(ctx, NK_TREE_NODE,
			node->getName().c_str(),
			NK_MINIMIZED, currtreenodeid)) {

			currtreenodeid++;

			if (selectedNode == node) {
				nk_label(ctx, "selected!", NK_TEXT_LEFT);
			} else {
				if (nk_button_label(ctx, "select")) {
					selectedNode = node;
					nodeSelected = true;
				}
			}

			std::vector<VESceneNode*> children = node->getChildrenList();

			for (int i = 0; i < children.size(); i++) {
				addToTree(children[i], ctx);
			}

			nk_tree_pop(ctx);
		}
	}
}