import os
import csv
import glob
import shutil
import subprocess
import re
import pandas as pd

TESTS = []
SOLUTIONS = []

DEFAULT_TIMEOUT = 10.0  # 초 (각 테스트 케이스 실행 제한)


def detect_language_by_extension(file_path):
    _, ext = os.path.splitext(file_path)
    ext = ext.lower()
    if ext == ".c":
        return "C"
    elif ext in [".cpp", ".cxx", ".cc"]:
        return "C++"
    elif ext == ".py":
        return "Python"
    else:
        return None

def detect_language_in_folder(folder_path):
    has_cpp = False
    has_c   = False
    has_py  = False
    for root, _, files in os.walk(folder_path):
        for f in files:
            lang = detect_language_by_extension(f)
            if lang == "C++":
                has_cpp = True
            elif lang == "C":
                has_c = True
            elif lang == "Python":
                has_py = True
    if has_cpp: return "C++"
    if has_c:   return "C"
    if has_py:  return "Python"
    return "Unknown"

def collect_sources(folder_path, lang):
    out = []
    for root, _, files in os.walk(folder_path):
        for f in files:
            ext = os.path.splitext(f.lower())[1]
            if lang == "C" and ext == ".c":
                out.append(os.path.join(root, f))
            elif lang == "C++" and ext in (".cpp", ".cxx", ".cc"):
                out.append(os.path.join(root, f))
    return out


def has_sources(folder_path: str) -> bool:
    for root, _, files in os.walk(folder_path):
        for f in files:
            ext = os.path.splitext(f.lower())[1]
            if ext in ('.c', '.cpp', '.cxx', '.cc', '.py'):
                return True
    return False

def find_submission_dirs(student_path: str):

    subs = []
    if has_sources(student_path):
        return [("", student_path)]
    for entry in os.listdir(student_path):
        p = os.path.join(student_path, entry)
        if os.path.isdir(p) and has_sources(p):
            subs.append((entry, p))
    return subs


def choose_compiler(lang):
    """
    macOS: Xcode Command Line Tools의 Clang 사용.
    (clang, clang++)
    """
    if lang == "C++":
        compiler = shutil.which("clang++")
        return ("clang", compiler if compiler else None)
    elif lang == "C":
        compiler = shutil.which("clang")
        return ("clang", compiler if compiler else None)
    return (None, None)

def find_latest_exe(build_dir):
    cands = glob.glob(os.path.join(build_dir, "**", "*"), recursive=True)
    cands = [p for p in cands if os.access(p, os.X_OK) and not os.path.isdir(p)]
    if not cands:
        return None
    cands.sort(key=lambda p: os.path.getmtime(p), reverse=True)
    return cands[0]

def build_c_cpp(folder_path, lang):


    build_dir = os.path.join(folder_path, "build")
    os.makedirs(build_dir, exist_ok=True)
    exe_path = os.path.join(build_dir, "main")
    if os.path.exists(exe_path):
        try:
            os.remove(exe_path)
        except PermissionError:
            print(f"⚠️ Warning: could not delete {exe_path}, it may still be running.")
    sources = collect_sources(folder_path, lang)

    if not sources:
        return None, "No source files found"

    toolchain, compiler = choose_compiler(lang)
    if not compiler:
        return None, "g++/gcc not found in PATH (install MinGW-w64 and add to PATH)"

    try:
        if lang == "C++":

            cmd = [compiler, "-std=c++20", "-O2",
                   "-Wall", "-Wextra", "-Wpedantic", "-Wshadow", "-Wconversion",
                   "-o", exe_path] + sources
        else:

            cmd = [compiler, "-std=c17", "-O2",
                   "-Wall", "-Wextra", "-Wpedantic", "-Wconversion", "-Wshadow",
                   "-o", exe_path] + sources


        proc = subprocess.run(
            cmd, cwd=build_dir, capture_output=True, text=True,
            encoding="utf-8", errors="replace"
        )
        if proc.returncode != 0:
            return None, f"Build failed:\n{proc.stdout}\n{proc.stderr}"

        if not os.path.exists(exe_path):
            exe = find_latest_exe(build_dir)
            if exe:
                exe_path = exe
            else:
                return None, "Executable not found after build"

        return exe_path, None
    except Exception as e:
        return None, f"Build error: {e}"


def get_python_cmd():
    for candidate in ["python3", "python", "py"]:
        py = shutil.which(candidate)
        if py:
            return [py]
    return None

def find_python_entry(folder_path):

    root_main = os.path.join(folder_path, "main.py")
    if os.path.exists(root_main):
        return root_main

    py_files = []
    for root, _, files in os.walk(folder_path):
        for f in files:
            if f.lower().endswith(".py"):
                py_files.append(os.path.join(root, f))

    py_files = [p for p in py_files if os.path.basename(p).lower() != "__init__.py"]

    if len(py_files) == 1:
        return py_files[0]

    return None

def run_python_once(folder_path, script_path, test_meta):
    """
    test_meta: {"path": <테스트파일경로>, "use_stdin": bool, "flags": [..]}
    """
    py_cmd = get_python_cmd()
    if not py_cmd:
        return None, "Python interpreter not found (install Python 3.x and add to PATH)"

    use_stdin = test_meta.get("use_stdin", True)
    path = test_meta["path"]
    flags = list(test_meta.get("flags", []))

    try:
        if use_stdin:
            with open(path, "rb") as fin:
                proc = subprocess.run(
                    py_cmd + [script_path] + flags,
                    stdin=fin,
                    capture_output=True, text=True, timeout=DEFAULT_TIMEOUT,
                    encoding="utf-8", errors="replace"
                )
        else:
            proc = subprocess.run(
                py_cmd + [script_path] + flags + [path],
                capture_output=True, text=True, timeout=DEFAULT_TIMEOUT,
                encoding="utf-8", errors="replace"
            )
        return (proc.stdout or "").strip(), None
    except subprocess.TimeoutExpired:
        return None, f"Timeout (>{DEFAULT_TIMEOUT}s)"
    except Exception as e:
        return None, f"Run error: {e}"


def run_executable_once(executable, test_meta):
    """
    test_meta: {"path": <테스트파일경로>, "use_stdin": bool, "flags": [..]}
    macOS: clang stdout flush 문제 대응 + locale 일관화
    """
    use_stdin = test_meta.get("use_stdin", True)
    path = test_meta["path"]
    flags = list(test_meta.get("flags", []))

    try:
        # 환경변수 보정: locale, flush 보장
        env = {
            **os.environ,
            "LC_ALL": "C",
            "LANG": "C",
            "STDOUT_FLUSH_ON_EXIT": "1"
        }

        if use_stdin:
            with open(path, "rb") as fin:
                proc = subprocess.run(
                    [executable] + flags,
                    stdin=fin,
                    capture_output=True, text=True, timeout=DEFAULT_TIMEOUT,
                    encoding="utf-8", errors="replace",
                    env=env
                )
        else:
            proc = subprocess.run(
                [executable] + flags + [path],
                capture_output=True, text=True, timeout=DEFAULT_TIMEOUT,
                encoding="utf-8", errors="replace",
                env=env
            )

        # 🔧 stdout + stderr 모두 확인 (clang 경고도 결과에 포함)
        out = (proc.stdout or "").strip()
        if proc.stderr:
            out += "\n" + proc.stderr.strip()

        # 🔧 혹시 flush 안 된 출력이 남는 경우 강제 flush
        out = out.replace("\r\n", "\n").replace("\r", "\n")
        return out, None

    except subprocess.TimeoutExpired:
        return None, f"Timeout (>{DEFAULT_TIMEOUT}s)"
    except Exception as e:
        return None, f"Run error: {e}"

def normalize_text(s):
    lines = s.replace("\r\n", "\n").replace("\r", "\n").split("\n")
    return [ln.rstrip() for ln in lines]

def canonicalize_for_compare(s: str) -> str:
    s = s.replace("\r\n", "\n").replace("\r", "\n")
    s = s.lower()
    s = re.sub(r"\s+", "", s)
    return s

def compare_output_with_solution(output, solution_file):
    with open(solution_file, 'r', encoding='utf-8') as f:
        solution = f.read()
    out_c = canonicalize_for_compare(output)
    sol_c = canonicalize_for_compare(solution)
    return 100 if out_c == sol_c else 0


def process_students_folders(current_folder):
    results = []

    this_script = os.path.basename(__file__)

    # 현재 폴더 내 .c, .cpp, .py 파일 처리
    code_files = [
        f for f in os.listdir(current_folder)
        if f.endswith((".c", ".cpp", ".py"))
        and f != this_script
    ]

    if not code_files:
        print("No source files (.c, .cpp, .py) found in this folder.")
        return results

    for code_file in code_files:
        student_name, ext = os.path.splitext(code_file)
        language = detect_language_by_extension(code_file)
        print(f"Processing {student_name} ({language}) ...")

        # 임시 build 폴더 생성
        temp_dir = os.path.join(current_folder, f"_build_{student_name}")
        os.makedirs(temp_dir, exist_ok=True)
        shutil.copy(os.path.join(current_folder, code_file), temp_dir)

        outputs, scores = [], []

        if language in ("C", "C++"):
            exe_path, build_err = build_c_cpp(temp_dir, language)
            if build_err:
                outputs = [f"Error: {build_err}"] * len(TESTS)
                scores  = [0] * len(TESTS)
            else:
                for i, test_meta in enumerate(TESTS):
                    out, err = run_executable_once(exe_path, test_meta)
                    if err:
                        outputs.append(f"Error: {err}")
                        scores.append(0)
                    else:
                        outputs.append(out)
                        scores.append(compare_output_with_solution(out, SOLUTIONS[i]))

        elif language == "Python":
            py_files = [
                f for f in os.listdir(temp_dir)
                if f.endswith(".py") and not f.startswith("__")
            ]
            if not py_files:
                outputs = ["Error: No Python file found"] * len(TESTS)
                scores = [0] * len(TESTS)
            else:
                entry = os.path.join(temp_dir, py_files[0])
                for i, test_meta in enumerate(TESTS):
                    out, err = run_python_once(temp_dir, entry, test_meta)
                    if err:
                        outputs.append(f"Error: {err}")
                        scores.append(0)
                    else:
                        outputs.append(out)
                        scores.append(compare_output_with_solution(out, SOLUTIONS[i]))
        else:
            outputs = ["Error: Unsupported or no language detected"] * len(TESTS)
            scores  = [0] * len(TESTS)

        results.append({
            'name': student_name,
            'language': language,
            'outputs': outputs,
            'scores': scores,
        })
        # shutil.rmtree(temp_dir, ignore_errors=True)

    return results

def save_results_to_csv(results, output_csv):
    num_tests = len(TESTS)
    header = ["Name", "Language"] + [f"Result{i+1}" for i in range(num_tests)] + [f"Score{i+1}" for i in range(num_tests)]
    with open(output_csv, mode='w', newline='', encoding='utf-8-sig') as file:
        writer = csv.writer(file)
        writer.writerow(header)
        for result in results:
            row = [result['name'], result['language']] + result['outputs'] + result['scores']
            writer.writerow(row)


def clean_text_for_sum(text):
    if text is None:
        return ""
    return re.sub(r'\s+', '', str(text)).strip().lower()

def calculate_similarity(result, reference_text):
    return 1 if result == reference_text else 0

def update_scores(results_csv, text_files):
    try:
        df = pd.read_csv(results_csv, encoding='utf-8-sig')

        result_cols = [c for c in df.columns if re.fullmatch(r"Result\d+", c)]
        result_cols.sort(key=lambda x: int(x.replace("Result", "")))

        score_cols  = [f"Score{idx}" for idx in range(1, len(result_cols) + 1)]
        for sc in score_cols:
            if sc not in df.columns:
                df[sc] = 0

        for index, row in df.iterrows():
            total_score = 0
            for i, rcol in enumerate(result_cols, start=1):
                scol = f"Score{i}"
                result_text = clean_text_for_sum(row.get(rcol, ""))

                testcase_file = text_files[i - 1]
                try:
                    with open(testcase_file, 'r', encoding='utf-8') as f:
                        reference_text = clean_text_for_sum(f.read())
                except Exception as e:
                    print(f"Error reading file {testcase_file}: {e}")
                    reference_text = ""

                score01 = calculate_similarity(result_text, reference_text)
                df.at[index, scol] = score01
                total_score += score01

            df.at[index, 'SUM'] = total_score

        df.to_csv('updated_results.csv', index=False, encoding='utf-8-sig')
        print("Scores updated and saved to 'updated_results.csv'.")

    except Exception as e:
        print(f"Error updating scores: {e}")

if __name__ == "__main__":
    # =========================
    # 소스 코드 내 테스트 케이스 및 정답 파일 경로 설정
    # =========================

    # 채점할 학생 제출물들이 들어있는 상위 폴더 경로
    current_folder = os.getcwd()
    output_csv = os.path.join(current_folder, "results.csv")

    def extract_num(filename):
        match = re.search(r'testcase_(\d+)', filename)
        return int(match.group(1)) if match else 0

    arguments_list = sorted(
        [p for p in glob.glob(os.path.join(current_folder, "testcase_*.txt")) if "_ans" not in p],
        key=extract_num
    )

    solution_files = sorted(
        glob.glob(os.path.join(current_folder, "testcase_*_ans.txt")),
        key=extract_num
    )

    TESTS = []
    for i, p in enumerate(arguments_list):
        # 특정 테스트 케이스에서만 verbose 플래그 사용 예시
        need_verbose = (i == 2)
        TESTS.append({
            "path": p,
            "use_stdin": False,
            "flags": ["-v"] if need_verbose else []
        })

    SOLUTIONS = solution_files

    results = process_students_folders(current_folder)
    save_results_to_csv(results, output_csv)
    print(f"Results saved to {output_csv}")

    update_scores(output_csv, SOLUTIONS)