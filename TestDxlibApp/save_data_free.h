
#pragma once
#include <vector>
#include <unordered_map>

//free���[�h�̂�����̋󂫃}�X��
struct SavaDataFreeOneBlank {
	std::unordered_map<int, int>answerResult;
	int nSolved = 0;
	int cursorPos = 1;
};

struct SavaDataFree {
	std::vector<SavaDataFreeOneBlank> blanks;
};


//�Ǝ��N���X�ɑ΂���V���A���C�Y��`(�e����hnn���O��Ԃɒ�`���邱��)
namespace hnn {
	template<class Arc>
	struct SerialFunc<Arc, SavaDataFree> {
		static void f(Arc& ar, SavaDataFree& sd) {
			serialize(ar, sd.blanks);
		}
	};//class SerialFunc {
	template<class Arc>
	struct SerialFunc<Arc, SavaDataFreeOneBlank> {
		static void f(Arc& ar, SavaDataFreeOneBlank& one) {
			serialize(ar, one.answerResult);
			serialize(ar, one.nSolved);
			serialize(ar, one.cursorPos);
		}
	};//class SerialFunc {
}

