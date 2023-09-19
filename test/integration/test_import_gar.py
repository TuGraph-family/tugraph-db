import pytest
import logging
from pathlib import Path

log = logging.getLogger(__name__)

class TestImportGar:
    config_path = Path.cwd().parent.parent / "test/resource/data/ldbc_parquet/ldbc_sample.graph.yml"

    IMPORTOPT = {"cmd":f"./lgraph_import -c {config_path} --gar true --overwrite true --d gar_db"}

    @pytest.mark.parametrize("importer", [IMPORTOPT], indirect=True)
    def test_cpp_client(self, importer):
        pass