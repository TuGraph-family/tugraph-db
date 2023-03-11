import pytest

EXECOPT = {
    "cmd" : "python3 embedded_api_unittest.py"
}

class TestEmbeddedApi:

    @pytest.mark.parametrize("exec", [EXECOPT], indirect=True)
    def test_embedded_api(self, exec):
        pass